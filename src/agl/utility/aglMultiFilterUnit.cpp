#include "utility/aglMultiFilterUnit.h"

#include <gfx/seadViewport.h>
#include <hostio/seadHostIOPropertyEvent.h>

#include "common/aglDrawContext.h"
#include "common/aglShaderProgram.h"
#include "detail/aglShaderHolder.h"
#include "postfx/aglColorCorrection.h"
#include "utility/aglDynamicTextureAllocator.h"
#include "utility/aglMultiFilter.h"
#include "utility/aglPrimitiveShape.h"
#include "utility/aglVertexAttributeHolder.h"

namespace agl::utl {

namespace {

const TextureFormat cFormats[] = {
    TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm,
    TextureFormat::cTextureFormat_R5_G6_B5_uNorm,
    TextureFormat::cTextureFormat_R8_G8_uNorm,
    TextureFormat::cTextureFormat_R8_uNorm,
    TextureFormat::cTextureFormat_R11_G11_B10_float,
};

inline TextureCompSel getCompSel(s32 value, TextureCompSel defaultCompSel)
{
    switch (value)
    {
    case 0:
        return cTextureCompSel_R;
    case 1:
        return cTextureCompSel_G;
    case 2:
        return cTextureCompSel_B;
    case 3:
        return cTextureCompSel_A;
    case 4:
        return cTextureCompSel_0;
    case 5:
        return cTextureCompSel_1;
    default:
        return defaultCompSel;
    }
}

template <typename T>
inline void setUniform(DrawContext* pDrawContext, const UniformLocation& rLocation,
                       const T& rValue)
{
    if (rLocation.isValid())
    {
        rLocation.setUniformNVN(pDrawContext, sizeof(T) / sizeof(u32), &rValue);
    }
}

inline void setUniform(DrawContext* pDrawContext, const UniformLocation& rLocation, f32 value)
{
    if (rLocation.isValid())
    {
        rLocation.setUniformNVN(pDrawContext, 1, &value);
    }
}

inline u32 getWidth(const TextureData& rTexture)
{
    u32 width = rTexture.getWidth();
    return width > 1 ? width : 1;
}

inline u32 getHeight(const TextureData& rTexture)
{
    s32 min = rTexture.getMinHeight_();
    s32 height = rTexture.getHeight();
    return min > height ? min : height;
}

inline void drawIndexStream(DrawContext* pDrawContext, const IndexStream& rStream)
{
    u32 count = rStream.getCount();

    if (count != 0)
    {
        NVNdrawPrimitive primitive = rStream.getPrimitiveType();
        NVNcommandBuffer* pCommandBuffer = pDrawContext->getNvnCommandBuffer();
        NVNbufferAddress address = nvnBufferGetAddress(rStream.getNvnBuffer());
        nvnCommandBufferDrawElements(pCommandBuffer, primitive, NVNindexType(rStream.getFormat()),
                                     count, address);
    }
}

inline void drawQuad(DrawContext* pDrawContext)
{
    VertexAttributeHolder::instance()
        ->getVertexAttribute(VertexAttributeHolder::cAttribute_Quad)
        .activate(pDrawContext);
    drawIndexStream(pDrawContext, PrimitiveShape::instance()->getQuadIndexStream());
}

inline void setRenderSize(MultiFilterDrawContext* pContext, const sead::Vector2f& rSize)
{
    sead::BoundBox2f area;
    area.set(sead::Vector2f::zero, rSize);
    pContext->mRenderBuffer.setPhysicalArea(area);
    pContext->mRenderBuffer.setVirtualSize(rSize);
}

inline void restoreCompSel(MultiFilterDrawContext* pContext)
{
    if (pContext->mIsChangeCompSel)
    {
        pContext->mSampler.setCompSel(cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B,
                                      static_cast<TextureCompSel>(pContext->mAlphaCompSel));
        pContext->mIsChangeCompSel = false;
    }
}

}  // namespace

/**
 * Returns the parameter name of a filter type.
 * @param type filter type
 * @return name used for the parameter list of the type
 */
sead::SafeString MultiFilterUnit::getFilterName(FilterType type)
{
    const sead::SafeString cNames[cFilterType_Num] = {
        "reduce", "expand", "blur", "color_correction", "change_format", "color_drift", "trimming",
    };

    return cNames[type];
}

/**
 * Returns the display label of a filter type.
 * @param type filter type
 * @return label shown for the type
 */
sead::SafeString MultiFilterUnit::getFilterLabel(FilterType type)
{
    const sead::SafeString cLabels[cFilterType_Num] = {
        "縮小フィルター", "拡大フィルター",   "ぼかしフィルター",
        "カラーコレクション", "フォーマット変更", "版ずれフィルター",
        "切り抜き",
    };

    return cLabels[type];
}

/**
 * Constructs a filter unit and registers its common parameters.
 * @param type filter type of the unit
 * @param index index of the unit among the units of the same type
 */
MultiFilterUnit::MultiFilterUnit(FilterType type, s32 index)
    : sead::TListNode<MultiFilterUnit*>(this), mType(type), mId((type + 1) << 8 | (index + 1))
{
    addObj(&mParamObj, "param_array");
}

/**
 * Destroys the filter unit.
 */
MultiFilterUnit::~MultiFilterUnit() = default;

/**
 * Creates a filter unit of the given type.
 * @param type filter type
 * @param index index of the unit among the units of the same type
 * @param pHeap heap to allocate the unit from
 * @return the created unit, or nullptr for an unknown type
 */
MultiFilterUnit* MultiFilterUnit::create(FilterType type, s32 index, sead::Heap* pHeap)
{
    switch (type)
    {
    case cFilterType_Reduce:
        return new (pHeap) ReduceFilter(index);
    case cFilterType_Expand:
        return new (pHeap) ExpandFilter(index);
    case cFilterType_Blur:
        return new (pHeap) BlurFilter(index);
    case cFilterType_ColorCorrection:
        return new (pHeap) ColorCorrectionFilter(index);
    case cFilterType_ChangeFormat:
        return new (pHeap) ChangeFormat(index);
    case cFilterType_ColorDrift:
        return new (pHeap) ColorDrift(index);
    case cFilterType_Trimming:
        return new (pHeap) Trimming(index);
    default:
        return nullptr;
    }
}

/**
 * Sets the owner of the unit and puts it in the free list.
 * @param pOwner filter owning the unit
 * @param pFreeList list of unused units of the same type
 * @param pHeap heap for the type specific initialization
 */
void MultiFilterUnit::initialize(MultiFilter* pOwner, sead::TList<MultiFilterUnit*>* pFreeList,
                                 sead::Heap* pHeap)
{
    mOwner = pOwner;
    mFreeList = pFreeList;
    pFreeList->pushBack(this);
    doInitialize_(pHeap);
}

/**
 * Returns the unit to the free list and destroys it.
 */
void MultiFilterUnit::destroy()
{
    mFreeList->pushFront(this);
    *mActive = false;
    doResetParameters_();
    doDestroy_();
    delete this;
}

/**
 * Inactivates the unit, returning it to the free list and resetting its parameters.
 */
void MultiFilterUnit::inactivate()
{
    mFreeList->pushFront(this);
    *mActive = false;
    doResetParameters_();
}

/**
 * Activates the unit, appending it to the active units of its owner.
 */
void MultiFilterUnit::activate()
{
    mOwner->mActiveUnits.pushBack(this);
    *mEnable = true;
    *mActive = true;
}

/**
 * Resets the parameters of the unit to their initial values.
 */
void MultiFilterUnit::resetParameters()
{
    doResetParameters_();
}

/**
 * Generates the host IO message of the unit.
 * @param pContext host IO context
 */
void MultiFilterUnit::genMessage(sead::hostio::Context* pContext)
{
    sead::FormatFixedSafeString<128> header("GroupHeader=%s設定", getFilterLabel(mType).cstr());
    {
        sead::FormatFixedSafeString<128> comment(
            "Comment=You can disable %s while leaving the settings the same by removing the "
            "check ",
            getFilterLabel(mType).cstr());
    }

    {
        sead::FormatFixedSafeString<128> comment("Comment=Reverts to %s initial value ",
                                                 getFilterLabel(mType).cstr());
    }

    doGenMessage_(pContext);
}

/**
 * Forwards a property event to the unit and resets its parameters when requested.
 * @param pEvent property event
 */
void MultiFilterUnit::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    doListenPropertyEvent_(pEvent);

    if (pEvent->getId() == reinterpret_cast<const void*>(50000))
    {
        doResetParameters_();
    }
}

/**
 * Does nothing.
 * @param pHeap unused
 */
void MultiFilterUnit::doInitialize_(sead::Heap* pHeap) {}

/**
 * Does nothing.
 */
void MultiFilterUnit::doDestroy_() {}

/**
 * Does nothing.
 * @param pInfo unused
 */
void MultiFilterUnit::doCalcResultInfo_(MultiFilterResultInfo* pInfo) const {}

/**
 * Does nothing.
 * @param pEvent unused
 */
void MultiFilterUnit::doListenPropertyEvent_(const sead::hostio::PropertyEvent* pEvent) {}

/**
 * Constructs a reduce filter.
 * @param index index of the unit among the reduce filters
 */
ReduceFilter::ReduceFilter(s32 index) : MultiFilterUnit(cFilterType_Reduce, index) {}

/**
 * Destroys the filter.
 */
ReduceFilter::~ReduceFilter() = default;

/**
 * Reduces the source by the configured scale, in at most two passes.
 * @param pDrawContext draw context
 * @param pContext state of the filter chain
 */
void ReduceFilter::doDraw_(DrawContext* pDrawContext, MultiFilterDrawContext* pContext) const
{
    switch (*mScale)
    {
    case 4:
        drawReduce_(pDrawContext, pContext, cFilterScale_Quarter);
        drawReduce_(pDrawContext, pContext, cFilterScale_Quarter);
        break;
    case 3:
        drawReduce_(pDrawContext, pContext, cFilterScale_Half);
        drawReduce_(pDrawContext, pContext, cFilterScale_Quarter);
        break;
    default:
        drawReduce_(pDrawContext, pContext, FilterScale(*mScale));
        break;
    }
}

/**
 * Draws the source reduced by a power of two into a new result texture.
 * @param pDrawContext draw context
 * @param pContext state of the filter chain
 * @param scale power of two to reduce by
 */
void ReduceFilter::drawReduce_(DrawContext* pDrawContext, MultiFilterDrawContext* pContext,
                               FilterScale scale) const
{
    TextureData* pPrevResult = pContext->mResultTexture;
    const TextureData* pSrc = pPrevResult ? pPrevResult : pContext->mSrcTexture;

    s32 width = getWidth(*pSrc) >> scale;
    width = width > 1 ? width : 1;
    s32 height = getHeight(*pSrc) >> scale;
    height = height > 1 ? height : 1;

    TextureData* pResult = pContext->mTextureCache.alloc(
        pDrawContext, "MultiFilterReduce", pContext->mFormat, width, height, 1, nullptr,
        DynamicTextureCache::AllocateType(0), true);
    pContext->mResultTexture = pResult;
    pContext->mRenderTarget.applyTextureData(*pResult);
    setRenderSize(pContext, sead::Vector2f(width, height));
    pContext->mSampler.applyTextureData(*pSrc);

    if (!DynamicTextureAllocator::instance()->isCacheEnabled())
    {
        pContext->mRenderBuffer.bind(pDrawContext);
        sead::Viewport viewport(pContext->mRenderBuffer);
        viewport.apply(pDrawContext, pContext->mRenderBuffer);

        const ShaderProgram* pProgram =
            detail::ShaderHolder::instance()
                ->getShaderProgramUnsafe(detail::ShaderHolder::cMultiFilterReduce)
                ->getVariation(scale);
        pProgram->activate(pDrawContext, true);
        setUniform(pDrawContext, pProgram->getUniformLocation(2), *mOffsetAdjust);
        const f32 invWidth = 1.0f / getWidth(*pSrc);
        setUniform(pDrawContext, pProgram->getUniformLocation(1),
                   sead::Vector2f(invWidth, invWidth));
        setUniform(pDrawContext, pProgram->getUniformLocation(0),
                   sead::Vector4f(0.0f, 0.0f, 1.0f, 1.0f));
        pContext->mSampler.activate(pDrawContext, pProgram->getSamplerLocationValidate(0), -1,
                                    false);
        drawQuad(pDrawContext);
        pContext->mRenderTarget.invalidateGPUCache(pDrawContext);
    }

    if (pPrevResult)
    {
        pContext->mTextureCache.free(pPrevResult);
    }

    restoreCompSel(pContext);
}

/**
 * Shrinks the result size by the configured scale.
 * @param pInfo result information to update
 */
void ReduceFilter::doCalcResultInfo_(MultiFilterResultInfo* pInfo) const
{
    s32 width = pInfo->mWidth >> *mScale;
    pInfo->mWidth = width > 1 ? width : 1;
    s32 height = pInfo->mHeight >> *mScale;
    pInfo->mHeight = height > 1 ? height : 1;
}

/**
 * Resets the parameters to their initial values.
 */
void ReduceFilter::doResetParameters_()
{
    *mScale = 2;
    *mOffsetAdjust = 0.0f;
}

/**
 * Generates the host IO message of the parameters.
 * @param pContext host IO context
 */
void ReduceFilter::doGenMessage_(sead::hostio::Context* pContext)
{
    mOffsetAdjust.genMessageParameter(pContext, "Min=-10, Max=10");
}

/**
 * Constructs an expand filter.
 * @param index index of the unit among the expand filters
 */
ExpandFilter::ExpandFilter(s32 index) : MultiFilterUnit(cFilterType_Expand, index) {}

/**
 * Destroys the filter.
 */
ExpandFilter::~ExpandFilter() = default;

/**
 * Doubles the source size once per configured step.
 * @param pDrawContext draw context
 * @param pContext state of the filter chain
 */
void ExpandFilter::doDraw_(DrawContext* pDrawContext, MultiFilterDrawContext* pContext) const
{
    for (s32 i = *mScale; i > 0; i--)
    {
        drawExpand_(pDrawContext, pContext, cFilterScale_Half);
    }
}

/**
 * Draws the source enlarged by a power of two into a new result texture.
 * @param pDrawContext draw context
 * @param pContext state of the filter chain
 * @param scale power of two to enlarge by
 */
void ExpandFilter::drawExpand_(DrawContext* pDrawContext, MultiFilterDrawContext* pContext,
                               FilterScale scale) const
{
    const TextureData* pOrigin = pContext->mSrcTexture;
    TextureData* pPrevResult = pContext->mResultTexture;
    const TextureData* pSrc = pPrevResult ? pPrevResult : pOrigin;

    s32 maxWidth = getWidth(*pOrigin);
    s32 width = s32(getWidth(*pSrc)) << scale;
    width = maxWidth < width ? maxWidth : width;
    s32 maxHeight = getHeight(*pOrigin);
    s32 height = s32(getHeight(*pSrc)) << scale;
    height = maxHeight < height ? maxHeight : height;

    TextureData* pResult = pContext->mTextureCache.alloc(
        pDrawContext, "MultiFilterExpand", pContext->mFormat, width, height, 1, nullptr,
        DynamicTextureCache::AllocateType(0), true);
    pContext->mResultTexture = pResult;
    pContext->mRenderTarget.applyTextureData(*pResult);
    setRenderSize(pContext, sead::Vector2f(width, height));
    pContext->mSampler.applyTextureData(*pSrc);

    if (!DynamicTextureAllocator::instance()->isCacheEnabled())
    {
        pContext->mRenderBuffer.bind(pDrawContext);
        sead::Viewport viewport(pContext->mRenderBuffer);
        viewport.apply(pDrawContext, pContext->mRenderBuffer);

        const ShaderProgram* pProgram =
            detail::ShaderHolder::instance()
                ->getShaderProgramUnsafe(detail::ShaderHolder::cMultiFilterExpand)
                ->getVariation(scale);
        pProgram->activate(pDrawContext, true);
        setUniform(pDrawContext, pProgram->getUniformLocation(2), *mOffsetAdjust);
        const f32 invWidth = 1.0f / getWidth(*pSrc);
        setUniform(pDrawContext, pProgram->getUniformLocation(1),
                   sead::Vector2f(invWidth, invWidth));
        setUniform(pDrawContext, pProgram->getUniformLocation(0),
                   sead::Vector4f(0.0f, 0.0f, 1.0f, 1.0f));
        pContext->mSampler.activate(pDrawContext, pProgram->getSamplerLocationValidate(0), -1,
                                    false);
        drawQuad(pDrawContext);
        pContext->mRenderTarget.invalidateGPUCache(pDrawContext);
    }

    if (pPrevResult)
    {
        pContext->mTextureCache.free(pPrevResult);
    }

    restoreCompSel(pContext);
}

/**
 * Grows the result size by the configured scale, limited to the maximum size.
 * @param pInfo result information to update
 */
void ExpandFilter::doCalcResultInfo_(MultiFilterResultInfo* pInfo) const
{
    s32 width = pInfo->mWidth << *mScale;
    pInfo->mWidth = pInfo->mMaxWidth < width ? pInfo->mMaxWidth : width;
    s32 height = pInfo->mHeight << *mScale;
    pInfo->mHeight = pInfo->mMaxHeight < height ? pInfo->mMaxHeight : height;
}

/**
 * Resets the parameters to their initial values.
 */
void ExpandFilter::doResetParameters_()
{
    *mScale = 2;
    *mOffsetAdjust = 0.0f;
}

/**
 * Generates the host IO message of the parameters.
 * @param pContext host IO context
 */
void ExpandFilter::doGenMessage_(sead::hostio::Context* pContext)
{
    mOffsetAdjust.genMessageParameter(pContext, "Min=-10, Max=10");
}

/**
 * Constructs a blur filter.
 * @param index index of the unit among the blur filters
 */
BlurFilter::BlurFilter(s32 index) : MultiFilterUnit(cFilterType_Blur, index) {}

/**
 * Destroys the filter.
 */
BlurFilter::~BlurFilter() = default;

/**
 * Blurs the source the configured number of times.
 * @param pDrawContext draw context
 * @param pContext state of the filter chain
 */
void BlurFilter::doDraw_(DrawContext* pDrawContext, MultiFilterDrawContext* pContext) const
{
    ImageFilter2D::GaussianKernel kernel;

    switch (*mGaussianKernel)
    {
    case 3:
        kernel = ImageFilter2D::GaussianKernel(0);
        break;
    case 5:
        kernel = ImageFilter2D::GaussianKernel(1);
        break;
    case 7:
        kernel = ImageFilter2D::GaussianKernel(2);
        break;
    case 11:
        kernel = ImageFilter2D::GaussianKernel(4);
        break;
    case 13:
        kernel = ImageFilter2D::GaussianKernel(5);
        break;
    default:
        kernel = ImageFilter2D::GaussianKernel(3);
        break;
    }

    for (s32 i = 0; i < *mBlurNum; i++)
    {
        drawBlur_(pDrawContext, pContext, BlurType(*mBlurType), kernel);
    }
}

/**
 * Blurs the source into a new result texture.
 * @param pDrawContext draw context
 * @param pContext state of the filter chain
 * @param blurType kind of blur to apply, where 4 applies the vertical and horizontal gaussian
 * @param kernel gaussian kernel used by the gaussian blur types
 */
void BlurFilter::drawBlur_(DrawContext* pDrawContext, MultiFilterDrawContext* pContext,
                           BlurType blurType, ImageFilter2D::GaussianKernel kernel) const
{
    TextureData* pPrevResult = pContext->mResultTexture;
    const TextureData* pSrc = pPrevResult ? pPrevResult : pContext->mSrcTexture;

    s32 width = getWidth(*pSrc);
    s32 height = getHeight(*pSrc);

    TextureData* pResult = pContext->mTextureCache.alloc(
        pDrawContext, "MultiFilterBlur", pContext->mFormat, width, height, 1, nullptr,
        DynamicTextureCache::AllocateType(0), true);
    pContext->mResultTexture = pResult;
    pContext->mRenderTarget.applyTextureData(*pResult);
    setRenderSize(pContext, sead::Vector2f(width, height));
    pContext->mSampler.applyTextureData(*pSrc);

    if (!DynamicTextureAllocator::instance()->isCacheEnabled())
    {
        pContext->mRenderBuffer.bind(pDrawContext);
        sead::Viewport viewport(pContext->mRenderBuffer);
        viewport.apply(pDrawContext, pContext->mRenderBuffer);

        switch (blurType)
        {
        case 1:
            ImageFilter2D::drawBlur(pDrawContext, pContext->mSampler, viewport,
                                    ImageFilter2D::BlurType(1), sead::Vector2f::zero,
                                    sead::Vector2f::ones, sead::Vector2f::zero);
            break;
        case 2:
        case 4:
            ImageFilter2D::drawGaussian(pDrawContext, pContext->mSampler, viewport, kernel, true,
                                        true, sead::Vector2f::zero);
            break;
        case 3:
            ImageFilter2D::drawGaussian(pDrawContext, pContext->mSampler, viewport, kernel, false,
                                        true, sead::Vector2f::zero);
            break;
        default:
            break;
        }

        pContext->mRenderTarget.invalidateGPUCache(pDrawContext);
    }

    if (pPrevResult)
    {
        pContext->mTextureCache.free(pPrevResult);
    }

    restoreCompSel(pContext);

    if (blurType == 4)
    {
        drawBlur_(pDrawContext, pContext, BlurType(3), kernel);
    }
}

/**
 * Resets the parameters to their initial values.
 */
void BlurFilter::doResetParameters_()
{
    *mBlurType = 4;
    *mBlurNum = 1;
    *mGaussianKernel = 9;
}

/**
 * Generates the host IO message of the parameters.
 * @param pContext host IO context
 */
void BlurFilter::doGenMessage_(sead::hostio::Context* pContext)
{
    mBlurNum.genMessageParameter(pContext, "Min=-0, Max=10");
}

/**
 * Constructs a color correction filter.
 * @param index index of the unit among the color correction filters
 */
ColorCorrectionFilter::ColorCorrectionFilter(s32 index)
    : MultiFilterUnit(cFilterType_ColorCorrection, index)
{
}

/**
 * Destroys the filter.
 */
ColorCorrectionFilter::~ColorCorrectionFilter() = default;

/**
 * Creates the color correction used by the filter and registers its parameters.
 * @param pHeap heap to allocate the color correction from
 */
void ColorCorrectionFilter::doInitialize_(sead::Heap* pHeap)
{
    mColorCorrection = new (pHeap) pfx::ColorCorrection();
    mColorCorrection->initialize(1, pHeap, false);
    mColorCorrection->setEnable(true);
    addList(mColorCorrection, "color_correction");
}

/**
 * Destroys the color correction used by the filter.
 */
void ColorCorrectionFilter::doDestroy_()
{
    if (mColorCorrection)
    {
        delete mColorCorrection;
        mColorCorrection = nullptr;
    }
}

/**
 * Applies the color correction to the source into a new result texture.
 * @param pDrawContext draw context
 * @param pContext state of the filter chain
 */
void ColorCorrectionFilter::doDraw_(DrawContext* pDrawContext,
                                    MultiFilterDrawContext* pContext) const
{
    if (!mColorCorrection)
    {
        return;
    }

    TextureData* pPrevResult = pContext->mResultTexture;
    const TextureData* pSrc = pPrevResult ? pPrevResult : pContext->mSrcTexture;
    pContext->mSampler.applyTextureData(*pSrc);
    s32 width = getWidth(*pSrc);
    s32 height = getHeight(*pSrc);

    if (!DynamicTextureAllocator::instance()->isCacheEnabled())
    {
        mColorCorrection->drawMap(pDrawContext);
    }

    TextureData* pResult = pContext->mTextureCache.alloc(
        pDrawContext, "MultiFilterColorCorrection", pContext->mFormat, width, height, 1, nullptr,
        DynamicTextureCache::AllocateType(0), true);
    pContext->mResultTexture = pResult;
    pContext->mRenderTarget.applyTextureData(*pResult);
    setRenderSize(pContext, sead::Vector2f(width, height));

    if (!DynamicTextureAllocator::instance()->isCacheEnabled())
    {
        pContext->mRenderBuffer.bind(pDrawContext);
        sead::Viewport viewport(pContext->mRenderBuffer);
        viewport.apply(pDrawContext, pContext->mRenderBuffer);
        mColorCorrection->draw(pDrawContext, 0, pContext->mRenderBuffer, pContext->mSampler);
        pContext->mRenderTarget.invalidateGPUCache(pDrawContext);
    }

    if (pPrevResult)
    {
        pContext->mTextureCache.free(pPrevResult);
    }

    restoreCompSel(pContext);
}

/**
 * Resets the parameters of the color correction.
 */
void ColorCorrectionFilter::doResetParameters_()
{
    if (mColorCorrection)
    {
        mColorCorrection->resetAll();
    }
}

/**
 * Generates the host IO message of the color correction parameters.
 * @param pContext host IO context
 */
void ColorCorrectionFilter::doGenMessage_(sead::hostio::Context* pContext)
{
    if (mColorCorrection)
    {
        mColorCorrection->genMessageParameters(pContext);
    }
}

/**
 * Forwards a property event to the color correction.
 * @param pEvent property event
 */
void ColorCorrectionFilter::doListenPropertyEvent_(const sead::hostio::PropertyEvent* pEvent)
{
    if (mColorCorrection)
    {
        mColorCorrection->listenPropertyEvent(pEvent);
    }
}

/**
 * Constructs a format change filter.
 * @param index index of the unit among the format change filters
 */
ChangeFormat::ChangeFormat(s32 index) : MultiFilterUnit(cFilterType_ChangeFormat, index) {}

/**
 * Destroys the filter.
 */
ChangeFormat::~ChangeFormat() = default;

/**
 * Sets the result format and the component selection of the following draws.
 * @param pDrawContext unused
 * @param pContext state of the filter chain
 */
void ChangeFormat::doDraw_(DrawContext* pDrawContext, MultiFilterDrawContext* pContext) const
{
    const TextureFormat format = u32(*mFormat) < 5 ? cFormats[*mFormat] : pContext->mFormat;
    const TextureCompSel r = getCompSel(*mCompSelR, cTextureCompSel_R);
    const TextureCompSel g = getCompSel(*mCompSelG, cTextureCompSel_G);
    const TextureCompSel b = getCompSel(*mCompSelB, cTextureCompSel_B);
    const TextureCompSel a = getCompSel(*mCompSelA, cTextureCompSel_A);
    pContext->mFormat = format;
    pContext->mSampler.setCompSel(r, g, b, a);
    pContext->mIsChangeCompSel = true;
}

/**
 * Sets the result format.
 * @param pInfo result information to update
 */
void ChangeFormat::doCalcResultInfo_(MultiFilterResultInfo* pInfo) const
{
    if (u32(*mFormat) < 5)
    {
        pInfo->mFormat = cFormats[*mFormat];
    }
}

/**
 * Resets the parameters to their initial values.
 */
void ChangeFormat::doResetParameters_()
{
    *mFormat = 4;
    *mCompSelR = 0;
    *mCompSelG = 1;
    *mCompSelB = 2;
    *mCompSelA = 3;
}

/**
 * Does nothing.
 * @param pContext unused
 */
void ChangeFormat::doGenMessage_(sead::hostio::Context* pContext) {}

/**
 * Constructs a color drift filter.
 * @param index index of the unit among the color drift filters
 */
ColorDrift::ColorDrift(s32 index) : MultiFilterUnit(cFilterType_ColorDrift, index) {}

/**
 * Destroys the filter.
 */
ColorDrift::~ColorDrift() = default;

/**
 * Shifts the color channels of the source into a new result texture.
 * @param pDrawContext draw context
 * @param pContext state of the filter chain
 */
void ColorDrift::doDraw_(DrawContext* pDrawContext, MultiFilterDrawContext* pContext) const
{
    TextureData* pPrevResult = pContext->mResultTexture;
    const TextureData* pSrc = pPrevResult ? pPrevResult : pContext->mSrcTexture;
    s32 width = getWidth(*pSrc);
    s32 height = getHeight(*pSrc);

    TextureData* pResult = pContext->mTextureCache.alloc(
        pDrawContext, "MultiFilterColorDrift", pContext->mFormat, width, height, 1, nullptr,
        DynamicTextureCache::AllocateType(0), true);
    pContext->mResultTexture = pResult;
    pContext->mRenderTarget.applyTextureData(*pResult);
    setRenderSize(pContext, sead::Vector2f(width, height));
    pContext->mSampler.applyTextureData(*pSrc);

    if (!DynamicTextureAllocator::instance()->isCacheEnabled())
    {
        pContext->mRenderBuffer.bind(pDrawContext);
        sead::Viewport viewport(pContext->mRenderBuffer);
        viewport.apply(pDrawContext, pContext->mRenderBuffer);
        ImageFilter2D::drawColorDrift(pDrawContext, pContext->mSampler, viewport, *mDriftR,
                                      *mDriftG, *mDriftB, sead::Vector2f::ones,
                                      sead::Vector2f::zero);
        pContext->mRenderTarget.invalidateGPUCache(pDrawContext);
    }

    if (pPrevResult)
    {
        pContext->mTextureCache.free(pPrevResult);
    }

    restoreCompSel(pContext);
}

/**
 * Resets the parameters to their initial values.
 */
void ColorDrift::doResetParameters_()
{
    *mDriftR = sead::Vector2f::zero;
    *mDriftG = sead::Vector2f::zero;
    *mDriftB = sead::Vector2f::zero;
}

/**
 * Generates the host IO message of the parameters.
 * @param pContext host IO context
 */
void ColorDrift::doGenMessage_(sead::hostio::Context* pContext)
{
    mDriftR.genMessageParameter(pContext, "Min=-10, Max=10");
    mDriftG.genMessageParameter(pContext, "Min=-10, Max=10");
    mDriftB.genMessageParameter(pContext, "Min=-10, Max=10");
}

/**
 * Constructs a trimming filter.
 * @param index index of the unit among the trimming filters
 */
Trimming::Trimming(s32 index) : MultiFilterUnit(cFilterType_Trimming, index) {}

/**
 * Destroys the filter.
 */
Trimming::~Trimming() = default;

/**
 * Cuts the configured area out of the source into a new result texture.
 * @param pDrawContext draw context
 * @param pContext state of the filter chain
 */
void Trimming::doDraw_(DrawContext* pDrawContext, MultiFilterDrawContext* pContext) const
{
    TextureData* pPrevResult = pContext->mResultTexture;
    const TextureData* pSrc = pPrevResult ? pPrevResult : pContext->mSrcTexture;
    const f32 srcWidth = getWidth(*pSrc);
    s32 width = mScale->x * srcWidth;
    width = width > 1 ? width : 1;
    const f32 srcHeight = getHeight(*pSrc);
    s32 height = mScale->y * srcHeight;
    height = height > 1 ? height : 1;

    TextureData* pResult = pContext->mTextureCache.alloc(
        pDrawContext, "MultiFilterTrimming", pContext->mFormat, width, height, 1, nullptr,
        DynamicTextureCache::AllocateType(0), true);
    pContext->mResultTexture = pResult;
    pContext->mRenderTarget.applyTextureData(*pResult);
    setRenderSize(pContext, sead::Vector2f(width, height));
    pContext->mSampler.applyTextureData(*pSrc);

    if (!DynamicTextureAllocator::instance()->isCacheEnabled())
    {
        pContext->mRenderBuffer.bind(pDrawContext);
        sead::Viewport viewport(pContext->mRenderBuffer);
        viewport.apply(pDrawContext, pContext->mRenderBuffer);
        const sead::Vector2f translate(-mCenter->x, -mCenter->y);
        ImageFilter2D::drawTextureTexCoord(pDrawContext, pContext->mSampler, viewport,
                                           sead::Vector2f::ones, 0.0f, translate,
                                           sead::Vector2f::ones, sead::Vector2f::zero);
        pContext->mRenderTarget.invalidateGPUCache(pDrawContext);
    }

    if (pPrevResult)
    {
        pContext->mTextureCache.free(pPrevResult);
    }

    restoreCompSel(pContext);
}

/**
 * Scales the result size by the trimming scale.
 * @param pInfo result information to update
 */
void Trimming::doCalcResultInfo_(MultiFilterResultInfo* pInfo) const
{
    const f32 srcWidth = pInfo->mWidth;
    s32 width = mScale->x * srcWidth;
    pInfo->mWidth = width > 1 ? width : 1;
    const f32 srcHeight = pInfo->mHeight;
    s32 height = mScale->y * srcHeight;
    pInfo->mHeight = height > 1 ? height : 1;
}

/**
 * Resets the parameters to their initial values.
 */
void Trimming::doResetParameters_()
{
    *mCenter = sead::Vector2f::zero;
    *mScale = sead::Vector2f(0.5f, 0.5f);
}

/**
 * Generates the host IO message of the parameters.
 * @param pContext host IO context
 */
void Trimming::doGenMessage_(sead::hostio::Context* pContext)
{
    mCenter.genMessageParameter(pContext, "Min=-1.0, Max=1.0");
    mScale.genMessageParameter(pContext, "Min=0.0, Max=1.0");
}

}  // namespace agl::utl

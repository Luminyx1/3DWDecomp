#include "lighting/aglSSII.h"

#include <hostio/seadHostIOPropertyEvent.h>
#include <prim/seadSafeString.h>

#include "common/aglDrawContext.h"
#include "common/aglShaderProgram.h"
#include "detail/aglShaderHolder.h"
#include "postfx/aglPostFxUtil.h"
#include "utility/aglDynamicTextureAllocator.h"

namespace agl::lght {

namespace {

void setUniformVec2(const UniformLocation& rLocation, DrawContext* pDrawContext,
                    const sead::Vector2f& rValue)
{
    rLocation.setUniform(pDrawContext, 2, &rValue);
}

void setUniformF32(DrawContext* pDrawContext, f32 value, const UniformLocation& rLocation)
{
    rLocation.setUniform(pDrawContext, value);
}

const TextureData* allocTexture(utl::DynamicTextureAllocator* pAllocator,
                                DrawContext* pDrawContext, const char* pName,
                                TextureFormat format, u32 width, u32 height, bool withoutContext)
{
    if (withoutContext)
    {
        return pAllocator->allocWithoutContext(pDrawContext, pName, format, width, height, 1,
                                               nullptr,
                                               utl::DynamicTextureAllocator::cAllocateType_0,
                                               true, false);
    }

    return pAllocator->alloc(pDrawContext, pName, format, width, height, 1, nullptr,
                             utl::DynamicTextureAllocator::cAllocateType_0, true, false);
}

}  // namespace

/**
 * Allocates a dynamic texture for this buffer and sets it up as a render target.
 * @param pDrawContext draw context used for the allocation
 * @param type buffer type, which selects the texture format
 * @param width texture width
 * @param height texture height
 * @param pName name of the allocated texture
 * @param withoutContext whether to allocate without a draw context
 */
void SSII::TexBuf::init(DrawContext* pDrawContext, BufType type, u32 width, u32 height,
                        const char* pName, bool withoutContext) const
{
    utl::DynamicTextureAllocator* pAllocator = utl::DynamicTextureAllocator::instance();

    switch (type)
    {
    case cBufType_Albedo:
    case cBufType_Light:
    case cBufType_Normal:
        init(pDrawContext,
             allocTexture(pAllocator, pDrawContext, pName,
                          TextureFormat::cTextureFormat_R11_G11_B10_float, width, height,
                          withoutContext),
             width, height);
        break;
    case cBufType_Depth:
        init(pDrawContext,
             allocTexture(pAllocator, pDrawContext, pName, TextureFormat::cTextureFormat_R32_float,
                          width, height, withoutContext),
             width, height);
        break;
    default:
        break;
    }

    mIsAllocated = true;
    mIsWithoutContext = withoutContext;
}

/**
 * Binds an existing texture to the sampler, render target, render buffer and viewport.
 * @param pDrawContext draw context (unused)
 * @param pTextureData texture to use
 * @param width buffer width
 * @param height buffer height
 */
void SSII::TexBuf::init(DrawContext* pDrawContext, const TextureData* pTextureData, u32 width,
                        u32 height) const
{
    setTextureData_(pDrawContext, pTextureData);
    mSampler.setFilter(1, 1, 1);
    mRenderTarget.applyTextureData(*mTextureData);
    mRenderBuffer.setVirtualSize(sead::Vector2f(width, height));
    mRenderBuffer.setPhysicalArea(sead::BoundBox2f(0.0f, 0.0f, width, height));
    mRenderBuffer.setRenderTargetColor(&mRenderTarget);
    mWidth = width;
    mHeight = height;
    mViewport.setByFrameBuffer(mRenderBuffer);
}

/**
 * Releases the texture if it was allocated by this buffer.
 */
void SSII::TexBuf::free() const
{
    if (mIsAllocated)
    {
        utl::DynamicTextureAllocator::instance()->free(mTextureData);
    }

    mTextureData = nullptr;
    mIsAllocated = false;
    mIsWithoutContext = false;
}

/**
 * Destroys the buffer, releasing its texture.
 */
SSII::TexBuf::~TexBuf()
{
    free();
}

/**
 * Releases the textures of all buffers of this reduction level.
 */
void SSII::ReduceBuffer::free() const
{
    for (const TexBuf& rBuf : mBuffers)
    {
        rBuf.free();
    }
}

/**
 * Releases all textures and frees the buffer array.
 */
SSII::ReduceBuffer::~ReduceBuffer()
{
    for (TexBuf& rBuf : mBuffers)
    {
        rBuf.free();
    }

    mBuffers.freeBuffer();
}

/**
 * Constructs the SSII renderer with the default quality settings.
 */
SSII::SSII()
    : mFlags(0), mReduceLevel{0, 1, 0}, mRefReduceLevel{0, 1, 0}, mDifLevelNum(7),
      mRefLevelNum(6), mRedBufLevel(3), mRefStartLevel(5), mDifIntensity(2.25f), mRefIntensity(3.76f),
      mDifSubIntensity(4.73f), mDifWeightStep(0.4f), mRefWeightStep(0.4f), mSteep(0.328f), mAntiHowlingThreshold(0.8f),
      mRefPow(25.0f), mRefInflate(1.0f), mRedBufQuality(2), mSampleQuality(2), mDifQuality(3),
      mRefQuality(2), mRefStartOffset(2), mResolutionLevel(10), mCurrentResolutionLevel(10)
{
}

/**
 * Releases all per-view buffers.
 */
SSII::~SSII()
{
    free();
    mRedBufSettings.freeBuffer();

    for (Context& rContext : mContexts)
    {
        rContext.mRefBuffers.freeBuffer();
        rContext.mDifBuffers.freeBuffer();
    }

    mContexts.freeBuffer();
}

/**
 * Releases the reprojection buffers of all views.
 */
void SSII::free()
{
    for (Context& rContext : mContexts)
    {
        rContext.mReprojectionBuffer.free();
    }
}

/**
 * Allocates the per-view contexts and reduction buffers and sets up the graphics contexts.
 * @param viewNum number of views
 * @param pHeap heap to allocate from
 */
void SSII::initialize(s32 viewNum, sead::Heap* pHeap)
{
    mFlags.set(0x3c);
    mContexts.tryAllocBuffer(viewNum, pHeap);

    for (auto it = mContexts.begin(), end = mContexts.end(); it != end; ++it)
    {
        Context& rContext = *it;
        rContext.mIndex = it.getIndex();
        rContext.mDifBuffers.tryAllocBuffer(10, pHeap);

        for (ReduceBuffer& rBuffer : rContext.mDifBuffers)
        {
            rBuffer.mBuffers.tryAllocBuffer(cBufType_Num, pHeap);
        }

        rContext.mRefBuffers.tryAllocBuffer(10, pHeap);

        for (ReduceBuffer& rBuffer : rContext.mRefBuffers)
        {
            rBuffer.mBuffers.tryAllocBuffer(cBufType_Num, pHeap);
        }

        rContext.mSrcBuffer.mBuffers.tryAllocBuffer(cBufType_Num, pHeap);
        rContext.mDifExpandBuffer.mBuffers.tryAllocBuffer(cBufType_Num, pHeap);
        rContext.mRefExpandBuffer.mBuffers.tryAllocBuffer(cBufType_Num, pHeap);
    }

    mRedBufSettings.tryAllocBuffer(10, pHeap);
    applyQualitySetting_();

    mGraphicsContext[0].setDepthEnable(false, false);
    mGraphicsContext[0].setBlendEnable(false);
    mGraphicsContext[1].setDepthEnable(false, false);
    mGraphicsContext[1].setBlendEnable(true);
    mGraphicsContext[1].setBlendEquation(0, 1);
    mGraphicsContext[1].setBlendFactor(0, 2, 2);
    mGraphicsContext[2].setBlendEnableMask(0);
    mGraphicsContext[2].setDepthEnable(false, false);
    mGraphicsContext[2].setColorMask(0xff);
    mGraphicsContext[3].setDepthEnable(false, false);
    mGraphicsContext[3].setBlendEnableMask(0xffffffff);
    mGraphicsContext[3].setBlendEquation(0, 1);
    mGraphicsContext[3].setBlendFactor(0, 2, 2);
    mGraphicsContext[3].setBlendEquation(1, 1);
    mGraphicsContext[3].setBlendFactor(1, 2, 2);
    mGraphicsContext[3].setColorMask(0xff);

    mDebugPage.setUp(viewNum, "SSII", pHeap);
    mDifDebugPage.setUp(viewNum, "SSII-Dif", pHeap);
    mRefDebugPage.setUp(viewNum, "SSII-Ref", pHeap);
}

void SSII::applyQualitySetting_()
{
    const s32 cRedBufLevelTable[] = {4, 3, 2, 1};
    const s32 cSampleNumTable[][10] = {
        {0, 0, 1, 1, 1, 1, 1, 1, 2, 2},
        {0, 1, 1, 1, 1, 1, 2, 2, 2, 2},
        {1, 1, 1, 1, 2, 2, 2, 2, 2, 2},
        {1, 1, 2, 2, 2, 2, 2, 2, 2, 2},
    };

    mRedBufLevel = cRedBufLevelTable[mRedBufQuality];

    for (s32 i = 0; i < 10; i++)
    {
        mRedBufSettings[i].mWidth = cSampleNumTable[mSampleQuality][i];
        mRedBufSettings[i].mHeight = cSampleNumTable[mSampleQuality][i];
    }

    for (s32 i = 0; i < 10; i++)
    {
        mRedBufSettings[i].mSampleNum = 2;
        mRedBufSettings[i].mMode = 0;
        mRedBufSettings[i].mIntensity = 1.0f;
    }

    for (s32 i = 0; i < 10; i++)
    {
        s32 diff = i - mRedBufLevel;
        mRedBufSettings[i].mRefLevel = diff == 0 ? 1 : (diff == 1 ? 2 : 3);
        mRedBufSettings[i].mRefMode = 1;
        mRedBufSettings[i].mRefIntensity = 1.0f;
    }

    mDifLevelNum = mResolutionLevel - cRedBufLevelTable[mDifQuality];
    mRefLevelNum = mResolutionLevel - cRedBufLevelTable[mRefQuality];

    if (mDifLevelNum <= 0)
    {
        mDifLevelNum = 1;
    }

    if (mRefLevelNum <= 0)
    {
        mRefLevelNum = 1;
    }

    s32 start = mDifLevelNum - mRefStartOffset;
    mRefStartLevel = start < 0 ? 0 : start;
}

void SSII::calcView(s32 view, const cull::ViewFrustumCulling& rViewFrustum)
{
    if (mFlags.isOff(1))
    {
        return;
    }

    Context& rContext = mContexts[view];
    rContext.mViewFrustum = rViewFrustum;
    rContext.mTanFovyHalf.set(rViewFrustum.mAspect * rViewFrustum.mTanHalfFovy,
                              rViewFrustum.mTanHalfFovy);
    rContext.mProjOffset.set(rViewFrustum.mOffset.x * 2.0f * rViewFrustum.mAspect *
                                 rViewFrustum.mTanHalfFovy,
                             rViewFrustum.mTanHalfFovy * (rViewFrustum.mOffset.y * 2.0f));

    if (mCurrentResolutionLevel != mResolutionLevel)
    {
        mResolutionLevel = mCurrentResolutionLevel;
        applyQualitySetting_();
    }
}

/**
 * Does nothing.
 */
void SSII::calcGPU() {}

/**
 * Does nothing.
 * @param view view index
 */
void SSII::calcViewGPU(s32 view) {}

/**
 * Draws diffuse indirect lighting.
 * @param pDrawContext draw context
 * @param view view index
 * @param rRenderBuffer render buffer receiving the diffuse result
 * @param rViewport viewport
 * @param rLight light buffer
 * @param rAlbedo albedo buffer
 * @param rNormal normal buffer
 * @param rDepth depth buffer
 */
void SSII::draw(DrawContext* pDrawContext, s32 view, const RenderBuffer& rRenderBuffer,
                const sead::Viewport& rViewport, const TextureData& rLight,
                const TextureData& rAlbedo, const TextureData& rNormal,
                const TextureData& rDepth) const
{
    draw_(pDrawContext, view, &rRenderBuffer, nullptr, rViewport, &rLight, nullptr, &rAlbedo,
          &rNormal, &rDepth);
}

void SSII::draw_(DrawContext* pDrawContext, s32 view, const RenderBuffer* pRenderBuffer,
                 const RenderBuffer* pRefRenderBuffer, const sead::Viewport& rViewport,
                 const TextureData* pLight, const TextureData* pLightAlbedo,
                 const TextureData* pAlbedo, const TextureData* pNormal,
                 const TextureData* pDepth) const
{
    bool isDifEnable = (pRenderBuffer != nullptr) & mFlags.isOn(1 << 4);
    bool isRefEnable = (pRefRenderBuffer != nullptr) & mFlags.isOn(1 << 3);

    if (mFlags.isOff(1))
    {
        return;
    }

    if (!isDifEnable && !isRefEnable)
    {
        return;
    }

    u32 width = rViewport.getSizeX();
    u32 height = rViewport.getSizeY();
    Context& rContext = mContexts[view];
    s32 difNum = mDifLevelNum;
    s32 refNum = mRefLevelNum > mDifLevelNum ? mDifLevelNum : mRefLevelNum;

    s32 resolutionLevel = 0;

    for (u32 w = width, h = height; h >= 2 && w >= 2; w >>= 1, h >>= 1)
    {
        resolutionLevel++;
    }

    mCurrentResolutionLevel = resolutionLevel;

    {
        u32 w = width;
        u32 h = height;

        for (s32 i = 0; i < difNum; i++)
        {
            h >>= 1;
            w >>= 1;

            if (h == 0 || w == 0)
            {
                refNum = refNum > i ? i : refNum;
                difNum = i;
                break;
            }

            rContext.mDifBuffers[i].mWidth = w;
            rContext.mDifBuffers[i].mHeight = h;

            if (i < refNum)
            {
                rContext.mRefBuffers[i].mWidth = w;
                rContext.mRefBuffers[i].mHeight = h;
            }
        }
    }

    s32 lastLevel = mRedBufLevel < difNum ? mRedBufLevel : difNum - 1;
    rContext.mReprojectionWidth = rContext.mDifBuffers[lastLevel].mWidth;
    rContext.mReprojectionHeight = rContext.mDifBuffers[lastLevel].mHeight;
    allocReprojectionBuffer_(pDrawContext, view);

    ReduceBuffer& rSrc = rContext.mSrcBuffer;

    if (mFlags.isOff(1 << 6))
    {
        if (rContext.mReprojectionBuffer.mIsAllocated)
        {
            rContext.mReprojectionBuffer.mRenderBuffer.clear(pDrawContext, 0, 1,
                                                             sead::Color4f::cBlack, 1.0f, 0);
        }

        if (pLightAlbedo != nullptr)
        {
            rSrc.mBuffers[cBufType_Light].init(pDrawContext, pLightAlbedo, width, height);
        }
        else
        {
            rSrc.mBuffers[cBufType_Light].init(pDrawContext, pLight, width, height);
        }

        rSrc.mBuffers[cBufType_Albedo].init(pDrawContext, pAlbedo, width, height);
    }
    else
    {
        rSrc.mBuffers[cBufType_Light].init(pDrawContext,
                                           rContext.mReprojectionBuffer.mTextureData,
                                           rContext.mReprojectionWidth,
                                           rContext.mReprojectionHeight);
        rSrc.mBuffers[cBufType_Albedo].init(pDrawContext,
                                            rContext.mReprojectionBuffer.mTextureData,
                                            rContext.mReprojectionWidth,
                                            rContext.mReprojectionHeight);
    }

    rSrc.mBuffers[cBufType_Normal].init(pDrawContext, pNormal, width, height);
    rSrc.mBuffers[cBufType_Depth].init(pDrawContext, pDepth, width, height);

    const TexBuf& rDifExpand = rContext.mDifExpandBuffer.mBuffers[cBufType_Light];
    const TexBuf& rRefExpand = rContext.mRefExpandBuffer.mBuffers[cBufType_Light];
    mGraphicsContext[0].apply(pDrawContext);

    {
        u32 w = width;
        u32 h = height;

        for (s32 i = 0; i < difNum; i++)
        {
            h >>= 1;
            w >>= 1;

            if (h == 0 || w == 0)
            {
                refNum = refNum > i ? i : refNum;
                difNum = i;
                break;
            }

            ReduceBuffer& rDif = rContext.mDifBuffers[i];
            rDif.mBuffers[cBufType_Light].init(pDrawContext, cBufType_Light, w, h, "difbuf_light",
                                               false);
            rDif.mBuffers[cBufType_Normal].init(pDrawContext, cBufType_Normal, w, h,
                                                "difbuf_normal", false);
            rDif.mBuffers[cBufType_Depth].init(pDrawContext, cBufType_Depth, w, h, "difbuf_depth",
                                               false);
            rDif.mBuffers[cBufType_Albedo].init(
                pDrawContext, rSrc.mBuffers[cBufType_Albedo].mTextureData, w, h);
            if (i != 0)
            {
                const ReduceBuffer& rPrev = rContext.mDifBuffers[i - 1];
                drawReduce_(pDrawContext, view, mReduceLevel[1],
                            rDif.mBuffers[cBufType_Normal].mRenderBuffer,
                            rPrev.mBuffers[cBufType_Normal].mSampler, false);
                drawReduce_(pDrawContext, view, mReduceLevel[2],
                            rDif.mBuffers[cBufType_Depth].mRenderBuffer,
                            rPrev.mBuffers[cBufType_Depth].mSampler, false);
            }
            else
            {
                drawReduce_(pDrawContext, view, 0, rDif.mBuffers[cBufType_Normal].mRenderBuffer,
                            rSrc.mBuffers[cBufType_Normal].mSampler, false);
                drawReduce_(pDrawContext, view, 0, rDif.mBuffers[cBufType_Depth].mRenderBuffer,
                            rSrc.mBuffers[cBufType_Depth].mSampler, false);
            }

            if (i < refNum)
            {
                ReduceBuffer& rRef = rContext.mRefBuffers[i];

                if (i != 0)
                {
                    const ReduceBuffer& rPrevRef = rContext.mRefBuffers[i - 1];

                    if (mReduceLevel[1] == mRefReduceLevel[1])
                    {
                        rRef.mBuffers[cBufType_Normal].init(
                            pDrawContext, rDif.mBuffers[cBufType_Normal].mTextureData, w, h);
                    }
                    else
                    {
                        rRef.mBuffers[cBufType_Normal].init(pDrawContext, cBufType_Normal, w, h,
                                                            "refbuf_normal", false);
                        drawReduce_(pDrawContext, view, mRefReduceLevel[1],
                                    rRef.mBuffers[cBufType_Normal].mRenderBuffer,
                                    rPrevRef.mBuffers[cBufType_Normal].mSampler, false);
                    }

                    if (mReduceLevel[2] == mRefReduceLevel[2])
                    {
                        rRef.mBuffers[cBufType_Depth].init(
                            pDrawContext, rDif.mBuffers[cBufType_Depth].mTextureData, w, h);
                    }
                    else
                    {
                        rRef.mBuffers[cBufType_Depth].init(pDrawContext, cBufType_Depth, w, h,
                                                           "refbuf_depth", false);
                        drawReduce_(pDrawContext, view, mRefReduceLevel[2],
                                    rRef.mBuffers[cBufType_Depth].mRenderBuffer,
                                    rPrevRef.mBuffers[cBufType_Depth].mSampler, false);
                    }
                }
                else
                {
                    rRef.mBuffers[cBufType_Normal].init(
                        pDrawContext, rDif.mBuffers[cBufType_Normal].mTextureData, w, h);
                    rRef.mBuffers[cBufType_Depth].init(
                        pDrawContext, rDif.mBuffers[cBufType_Depth].mTextureData, w, h);
                }

                rRef.mBuffers[cBufType_Albedo].init(
                    pDrawContext, rDif.mBuffers[cBufType_Albedo].mTextureData, w, h);
            }
        }
    }

    s32 expandLevel = lastLevel > difNum ? difNum : lastLevel;

    for (s32 i = 0; i < expandLevel; i++)
    {
        ReduceBuffer& rRef = rContext.mRefBuffers[i];
        const TexBuf& rNormal = rRef.mBuffers[cBufType_Normal];
        rRef.mBuffers[cBufType_Light].init(pDrawContext, cBufType_Light, rNormal.mWidth,
                                           rNormal.mHeight, "refbuf_light", false);
    }

    for (s32 i = expandLevel; i < difNum; i++)
    {
        ReduceBuffer& rDif = rContext.mDifBuffers[i];

        if (i != expandLevel)
        {
            mGraphicsContext[0].apply(pDrawContext);
            drawReduce_(pDrawContext, view, mReduceLevel[0],
                        rDif.mBuffers[cBufType_Light].mRenderBuffer,
                        rContext.mDifBuffers[i - 1].mBuffers[cBufType_Light].mSampler, false);
        }
        else
        {
            const TexBuf& rLight = rDif.mBuffers[cBufType_Light];

            if (mFlags.isOff(1 << 6))
            {
                drawReduceWithPreRender_(pDrawContext, view, 0, rLight.mRenderBuffer,
                                         rSrc.mBuffers[cBufType_Light].mSampler,
                                         rSrc.mBuffers[cBufType_Albedo].mSampler);
            }
            else
            {
                rLight.free();
                rLight.init(pDrawContext, rSrc.mBuffers[cBufType_Light].mTextureData,
                            rContext.mReprojectionWidth, rContext.mReprojectionHeight);
            }
        }
    }

    const TexBuf& rLastLight = rContext.mDifBuffers[expandLevel].mBuffers[cBufType_Light];

    if (isDifEnable)
    {
        ReduceBuffer& rExpand = rContext.mDifExpandBuffer;
        rExpand.mBuffers[cBufType_Light].init(pDrawContext, cBufType_Light, rLastLight.mWidth,
                                              rLastLight.mHeight, "dif_renderbuf", false);
        rExpand.mBuffers[cBufType_Albedo].init(pDrawContext,
                                               rSrc.mBuffers[cBufType_Albedo].mTextureData,
                                               rLastLight.mWidth, rLastLight.mHeight);
        rExpand.mBuffers[cBufType_Normal].init(pDrawContext,
                                               rSrc.mBuffers[cBufType_Normal].mTextureData,
                                               rLastLight.mWidth, rLastLight.mHeight);
        rExpand.mBuffers[cBufType_Depth].init(pDrawContext,
                                              rSrc.mBuffers[cBufType_Depth].mTextureData,
                                              rLastLight.mWidth, rLastLight.mHeight);
        rExpand.mBuffers[cBufType_Light].mRenderBuffer.bind(pDrawContext);
        rExpand.mBuffers[cBufType_Light].mViewport.apply(
            pDrawContext, rExpand.mBuffers[cBufType_Light].mRenderBuffer);
        rExpand.mBuffers[cBufType_Light].mRenderBuffer.clear(pDrawContext, 0, 1,
                                                             sead::Color4f::cBlack, 1.0f, 0);
    }

    if (isRefEnable)
    {
        ReduceBuffer& rExpand = rContext.mRefExpandBuffer;
        rExpand.mBuffers[cBufType_Light].init(pDrawContext, cBufType_Light, rLastLight.mWidth,
                                              rLastLight.mHeight, "ref_renderbuf", false);
        rExpand.mBuffers[cBufType_Albedo].init(pDrawContext,
                                               rSrc.mBuffers[cBufType_Albedo].mTextureData,
                                               rLastLight.mWidth, rLastLight.mHeight);
        rExpand.mBuffers[cBufType_Normal].init(pDrawContext,
                                               rSrc.mBuffers[cBufType_Normal].mTextureData,
                                               rLastLight.mWidth, rLastLight.mHeight);
        rExpand.mBuffers[cBufType_Depth].init(pDrawContext,
                                              rSrc.mBuffers[cBufType_Depth].mTextureData,
                                              rLastLight.mWidth, rLastLight.mHeight);
        rExpand.mBuffers[cBufType_Light].mRenderBuffer.bind(pDrawContext);
        rExpand.mBuffers[cBufType_Light].mViewport.apply(
            pDrawContext, rExpand.mBuffers[cBufType_Light].mRenderBuffer);
        rExpand.mBuffers[cBufType_Light].mRenderBuffer.clear(pDrawContext, 0, 1,
                                                             sead::Color4f::cBlack, 1.0f, 0);
    }

    mGraphicsContext[1].apply(pDrawContext);

    if (isDifEnable)
    {
        const ReduceBuffer& rLastDif = rContext.mDifBuffers[expandLevel];
        f32 weight = 1.0f;

        for (s32 i = difNum - 1; i >= expandLevel; i--)
        {
            const ReduceBuffer& rDif = rContext.mDifBuffers[i];
            weight += mDifWeightStep;
            RedBufRenderSetting setting = mRedBufSettings[i];
            setting.mIntensity = setting.mIntensity * (mDifIntensity / weight);
            drawSSII_(pDrawContext, view, setting, rDifExpand.mRenderBuffer, rDifExpand.mViewport,
                      rDif, rLastDif, false);
            if (i != expandLevel && i >= mRefStartLevel && mFlags.isOn(1 << 2))
            {
                const ReduceBuffer& rPrev = rContext.mDifBuffers[i - 1];
                const TexBuf& rPrevLight = rPrev.mBuffers[cBufType_Light];
                setting = mRedBufSettings[i];
                setting.mIntensity = mDifSubIntensity * setting.mIntensity;
                drawSSII_(pDrawContext, view, setting, rPrevLight.mRenderBuffer,
                          rPrevLight.mViewport, rDif, rPrev, true);
            }
        }
    }

    if (isRefEnable)
    {
        const ReduceBuffer& rLastRef = rContext.mRefBuffers[expandLevel];
        f32 weight = 1.0f;

        for (s32 i = refNum - 1; i >= expandLevel; i--)
        {
            const ReduceBuffer& rRef = rContext.mRefBuffers[i];
            const ReduceBuffer& rDif = rContext.mDifBuffers[i];
            weight += mRefWeightStep;
            const RedBufRenderSetting& rSetting = mRedBufSettings[i];
            drawRef_(pDrawContext, view, rSetting.mRefIntensity * mRefIntensity / weight,
                     rSetting.mRefLevel, rSetting.mRefMode, rRefExpand.mRenderBuffer,
                     rRefExpand.mViewport, rDif, rRef, rLastRef);
        }
    }

    if (mFlags.isOn(1 << 7))
    {
        if (isDifEnable)
        {
            u32 w = rDifExpand.mWidth >> 1;
            u32 h = rDifExpand.mHeight >> 1;
            TexBuf buffer0;
            TexBuf buffer1;
            w = w != 0 ? w : 1;
            h = h != 0 ? h : 1;
            mGraphicsContext[0].apply(pDrawContext);
            buffer0.init(pDrawContext, cBufType_Light, w, h, "summary0", false);
            drawReduce_(pDrawContext, view, 0, buffer0.mRenderBuffer, rDifExpand.mSampler, false);
            TexBuf* pSrc = &buffer0;
            TexBuf* pDst = &buffer1;

            while (w != 1 || h != 1)
            {
                w = w != 1 ? w >> 1 : 1;
                h = h != 1 ? h >> 1 : 1;
                pDst->init(pDrawContext, cBufType_Light, w, h, "summary", false);
                drawReduce_(pDrawContext, view, 0, pDst->mRenderBuffer, pSrc->mSampler, false);
                pSrc->free();
                TexBuf* pTmp = pSrc;
                pSrc = pDst;
                pDst = pTmp;
            }

            drawAntiHowling_(pDrawContext, view, rDifExpand.mRenderBuffer, rDifExpand.mSampler,
                             pSrc->mSampler);
            pSrc->free();
        }

        if (isRefEnable)
        {
            u32 w = rRefExpand.mWidth >> 1;
            u32 h = rRefExpand.mHeight >> 1;
            TexBuf buffer0;
            TexBuf buffer1;
            w = w != 0 ? w : 1;
            h = h != 0 ? h : 1;
            mGraphicsContext[0].apply(pDrawContext);
            buffer0.init(pDrawContext, cBufType_Light, w, h, "summary0", false);
            drawReduce_(pDrawContext, view, 0, buffer0.mRenderBuffer, rRefExpand.mSampler, false);
            TexBuf* pSrc = &buffer0;
            TexBuf* pDst = &buffer1;

            while (w != 1 || h != 1)
            {
                w = w != 1 ? w >> 1 : 1;
                h = h != 1 ? h >> 1 : 1;
                pDst->init(pDrawContext, cBufType_Light, w, h, "summary", false);
                drawReduce_(pDrawContext, view, 0, pDst->mRenderBuffer, pSrc->mSampler, false);
                pSrc->free();
                TexBuf* pTmp = pSrc;
                pSrc = pDst;
                pDst = pTmp;
            }

            drawAntiHowling_(pDrawContext, view, rRefExpand.mRenderBuffer, rRefExpand.mSampler,
                             pSrc->mSampler);
            pSrc->free();
        }
    }

    if (isDifEnable && isRefEnable && mFlags.isOn(1 << 5))
    {
        for (s32 i = expandLevel; i >= 0; i--)
        {
            const ReduceBuffer& rDif = rContext.mDifBuffers[i];
            const TexBuf* pDifLight;
            const TexBuf* pRefLight;

            if (i == expandLevel)
            {
                pDifLight = &rDifExpand;
                pRefLight = &rRefExpand;
            }
            else
            {
                pDifLight = &rDif.mBuffers[cBufType_Light];
                pRefLight = &rContext.mRefBuffers[i].mBuffers[cBufType_Light];
            }

            if (i != 0)
            {
                const ReduceBuffer& rPrevDif = rContext.mDifBuffers[i - 1];
                const ReduceBuffer& rPrevRef = rContext.mRefBuffers[i - 1];
                rContext.mRenderBuffer.setVirtualSize(
                    rPrevDif.mBuffers[cBufType_Light].mRenderBuffer.getVirtualSize());
                rContext.mRenderBuffer.setPhysicalArea(
                    rPrevDif.mBuffers[cBufType_Light].mRenderBuffer.getPhysicalArea());
                rContext.mRenderBuffer.setRenderTargetColorNullAll();
                rContext.mRenderBuffer.setRenderTargetDepth(nullptr);
                rContext.mRenderBuffer.setRenderTargetColor(
                    &rPrevDif.mBuffers[cBufType_Light].mRenderTarget, 0);
                rContext.mRenderBuffer.setRenderTargetColor(
                    &rPrevRef.mBuffers[cBufType_Light].mRenderTarget, 1);
                mGraphicsContext[2].apply(pDrawContext);
                drawExpand_(pDrawContext, view, rContext.mRenderBuffer, pDifLight->mSampler,
                            rDif.mBuffers[cBufType_Normal].mSampler,
                            rDif.mBuffers[cBufType_Depth].mSampler,
                            rPrevDif.mBuffers[cBufType_Normal].mSampler,
                            rPrevDif.mBuffers[cBufType_Depth].mSampler, nullptr,
                            &pRefLight->mSampler, mRedBufSettings[i].mWidth, false);
            }
            else if (pLightAlbedo != nullptr)
            {
                mGraphicsContext[1].apply(pDrawContext);
                drawExpand_(pDrawContext, view, *pRenderBuffer, pDifLight->mSampler,
                            rDif.mBuffers[cBufType_Normal].mSampler,
                            rDif.mBuffers[cBufType_Depth].mSampler,
                            rSrc.mBuffers[cBufType_Normal].mSampler,
                            rSrc.mBuffers[cBufType_Depth].mSampler,
                            &rSrc.mBuffers[cBufType_Albedo].mSampler, &pRefLight->mSampler,
                            mRedBufSettings[0].mWidth, false);
            }
            else
            {
                const RenderTargetColor* pDifTarget = pRenderBuffer->getRenderTargetColor();
                RenderTargetColor difTarget(*pDifTarget, pDifTarget->getMipLevel(),
                                            pDifTarget->getSlice());
                const RenderTargetColor* pRefTarget = pRefRenderBuffer->getRenderTargetColor();
                RenderTargetColor refTarget(*pRefTarget, pRefTarget->getMipLevel(),
                                            pRefTarget->getSlice());
                rContext.mRenderBuffer.setVirtualSize(pRenderBuffer->getVirtualSize());
                rContext.mRenderBuffer.setPhysicalArea(pRenderBuffer->getPhysicalArea());
                rContext.mRenderBuffer.setRenderTargetColorNullAll();
                rContext.mRenderBuffer.setRenderTargetColor(&difTarget, 0);
                rContext.mRenderBuffer.setRenderTargetColor(&refTarget, 1);
                rContext.mRenderBuffer.setRenderTargetDepth(nullptr);
                mGraphicsContext[3].apply(pDrawContext);
                drawExpand_(pDrawContext, view, rContext.mRenderBuffer, pDifLight->mSampler,
                            rDif.mBuffers[cBufType_Normal].mSampler,
                            rDif.mBuffers[cBufType_Depth].mSampler,
                            rSrc.mBuffers[cBufType_Normal].mSampler,
                            rSrc.mBuffers[cBufType_Depth].mSampler, nullptr, &pRefLight->mSampler,
                            mRedBufSettings[0].mWidth, false);
            }
        }
    }
    else
    {
        if (isDifEnable)
        {
            for (s32 i = expandLevel; i >= 0; i--)
            {
                const ReduceBuffer& rDif = rContext.mDifBuffers[i];
                const TexBuf& rLight =
                    i == expandLevel ? rDifExpand : rDif.mBuffers[cBufType_Light];
                if (i != 0)
                {
                    const ReduceBuffer& rPrev = rContext.mDifBuffers[i - 1];
                    mGraphicsContext[0].apply(pDrawContext);
                    drawExpand_(pDrawContext, view, rPrev.mBuffers[cBufType_Light].mRenderBuffer,
                                rLight.mSampler, rDif.mBuffers[cBufType_Normal].mSampler,
                                rDif.mBuffers[cBufType_Depth].mSampler,
                                rPrev.mBuffers[cBufType_Normal].mSampler,
                                rPrev.mBuffers[cBufType_Depth].mSampler, nullptr, nullptr,
                                mRedBufSettings[i].mWidth, false);
                }
                else
                {
                    mGraphicsContext[1].apply(pDrawContext);
                    drawExpand_(pDrawContext, view, *pRenderBuffer, rLight.mSampler,
                                rDif.mBuffers[cBufType_Normal].mSampler,
                                rDif.mBuffers[cBufType_Depth].mSampler,
                                rSrc.mBuffers[cBufType_Normal].mSampler,
                                rSrc.mBuffers[cBufType_Depth].mSampler,
                                pLightAlbedo != nullptr ?
                                    &rSrc.mBuffers[cBufType_Albedo].mSampler :
                                    nullptr,
                                nullptr, mRedBufSettings[0].mWidth, false);
                }
            }
        }

        if (isRefEnable)
        {
            for (s32 i = expandLevel; i >= 0; i--)
            {
                const ReduceBuffer& rRef = rContext.mRefBuffers[i];
                const TexBuf& rLight =
                    i == expandLevel ? rRefExpand : rRef.mBuffers[cBufType_Light];
                if (i != 0)
                {
                    const ReduceBuffer& rPrev = rContext.mRefBuffers[i - 1];
                    mGraphicsContext[0].apply(pDrawContext);
                    drawExpand_(pDrawContext, view, rPrev.mBuffers[cBufType_Light].mRenderBuffer,
                                rLight.mSampler, rRef.mBuffers[cBufType_Normal].mSampler,
                                rRef.mBuffers[cBufType_Depth].mSampler,
                                rPrev.mBuffers[cBufType_Normal].mSampler,
                                rPrev.mBuffers[cBufType_Depth].mSampler, nullptr, nullptr,
                                mRedBufSettings[i].mHeight, false);
                }
                else
                {
                    mGraphicsContext[1].apply(pDrawContext);
                    drawExpand_(pDrawContext, view, *pRefRenderBuffer, rLight.mSampler,
                                rRef.mBuffers[cBufType_Normal].mSampler,
                                rRef.mBuffers[cBufType_Depth].mSampler,
                                rSrc.mBuffers[cBufType_Normal].mSampler,
                                rSrc.mBuffers[cBufType_Depth].mSampler, nullptr, nullptr,
                                mRedBufSettings[0].mHeight, pLightAlbedo != nullptr);
                }
            }
        }
    }

    if (isDifEnable)
    {
        for (s32 i = 0; i < expandLevel; i++)
        {
            sead::FormatFixedSafeString<256> name("dif_expand_%d", i);
        }

        for (s32 i = expandLevel; i < difNum; i++)
        {
            sead::FormatFixedSafeString<256> name("dif_light_%d", i);
        }
    }

    if (isRefEnable)
    {
        for (s32 i = 0; i < expandLevel; i++)
        {
            sead::FormatFixedSafeString<256> name("ref_expand_%d", i);
        }
    }

    rSrc.free();

    for (s32 i = 0; i < 10; i++)
    {
        rContext.mDifBuffers[i].free();
        rContext.mRefBuffers[i].free();
    }

    rContext.mDifExpandBuffer.free();
    rContext.mRefExpandBuffer.free();
}

/**
 * Draws diffuse and reflected indirect lighting.
 * @param pDrawContext draw context
 * @param view view index
 * @param rRenderBuffer render buffer receiving the diffuse result
 * @param rRefRenderBuffer render buffer receiving the reflection result
 * @param rViewport viewport
 * @param rLight light buffer
 * @param rAlbedo albedo buffer
 * @param rNormal normal buffer
 * @param rDepth depth buffer
 */
void SSII::draw(DrawContext* pDrawContext, s32 view, const RenderBuffer& rRenderBuffer,
                const RenderBuffer& rRefRenderBuffer, const sead::Viewport& rViewport,
                const TextureData& rLight, const TextureData& rAlbedo,
                const TextureData& rNormal, const TextureData& rDepth) const
{
    draw_(pDrawContext, view, &rRenderBuffer, &rRefRenderBuffer, rViewport, &rLight, nullptr,
          &rAlbedo, &rNormal, &rDepth);
}

/**
 * Draws diffuse indirect lighting using the reprojection buffer as light source.
 * @param pDrawContext draw context
 * @param view view index
 * @param rRenderBuffer render buffer receiving the diffuse result
 * @param rViewport viewport
 * @param rNormal normal buffer
 * @param rDepth depth buffer
 */
void SSII::draw(DrawContext* pDrawContext, s32 view, const RenderBuffer& rRenderBuffer,
                const sead::Viewport& rViewport, const TextureData& rNormal,
                const TextureData& rDepth) const
{
    draw_(pDrawContext, view, &rRenderBuffer, nullptr, rViewport, nullptr, nullptr, nullptr,
          &rNormal, &rDepth);
}

/**
 * Draws diffuse and reflected indirect lighting using the reprojection buffer as light source.
 * @param pDrawContext draw context
 * @param view view index
 * @param rRenderBuffer render buffer receiving the diffuse result
 * @param rRefRenderBuffer render buffer receiving the reflection result
 * @param rViewport viewport
 * @param rNormal normal buffer
 * @param rDepth depth buffer
 */
void SSII::draw(DrawContext* pDrawContext, s32 view, const RenderBuffer& rRenderBuffer,
                const RenderBuffer& rRefRenderBuffer, const sead::Viewport& rViewport,
                const TextureData& rNormal, const TextureData& rDepth) const
{
    draw_(pDrawContext, view, &rRenderBuffer, &rRefRenderBuffer, rViewport, nullptr, nullptr,
          nullptr, &rNormal, &rDepth);
}

/**
 * Draws indirect lighting from a buffer that already contains light multiplied by albedo.
 * @param pDrawContext draw context
 * @param view view index
 * @param rRenderBuffer render buffer receiving the result
 * @param rViewport viewport
 * @param rLight light buffer multiplied by albedo
 * @param rAlbedo albedo buffer
 * @param rNormal normal buffer
 * @param rDepth depth buffer
 * @param refOnly whether to skip the reflection pass
 */
void SSII::drawAlbedoMode(DrawContext* pDrawContext, s32 view, const RenderBuffer& rRenderBuffer,
                          const sead::Viewport& rViewport, const TextureData& rLight,
                          const TextureData& rAlbedo, const TextureData& rNormal,
                          const TextureData& rDepth, bool refOnly) const
{
    if (refOnly)
    {
        draw_(pDrawContext, view, &rRenderBuffer, nullptr, rViewport, nullptr, &rLight, &rAlbedo,
              &rNormal, &rDepth);
    }
    else
    {
        draw_(pDrawContext, view, &rRenderBuffer, &rRenderBuffer, rViewport, nullptr, &rLight,
              &rAlbedo, &rNormal, &rDepth);
    }
}

/**
 * (Re)allocates the reprojection buffer when reprojection is enabled and its size changed.
 * @param pDrawContext draw context
 * @param view view index
 * @return whether a new buffer was allocated
 */
bool SSII::allocReprojectionBuffer_(DrawContext* pDrawContext, s32 view) const
{
    const Context& rContext = mContexts[view];
    const TexBuf& rBuffer = rContext.mReprojectionBuffer;

    if (mFlags.isOff(1 << 6))
    {
        rBuffer.free();
        return false;
    }

    if (rBuffer.mIsAllocated)
    {
        if (rBuffer.mWidth == rContext.mReprojectionWidth &&
            rBuffer.mHeight == rContext.mReprojectionHeight)
        {
            return false;
        }
    }

    rBuffer.free();
    rBuffer.init(pDrawContext, cBufType_Albedo, rContext.mReprojectionWidth,
                 rContext.mReprojectionHeight, "PrevFrameBuffer", true);
    rBuffer.mRenderBuffer.bind(pDrawContext);
    rBuffer.mRenderBuffer.clear(pDrawContext, 0, 1, sead::Color4f::cBlack, 1.0f, 0);
    return true;
}

/**
 * Downsamples a texture into a render buffer.
 * @param pDrawContext draw context
 * @param view view index (unused)
 * @param level reduction shader level
 * @param rRenderBuffer destination render buffer
 * @param rSampler source sampler
 * @param isDepth whether to use the depth variation of the reduce shader
 */
void SSII::drawReduce_(DrawContext* pDrawContext, s32 view, s32 level,
                       const RenderBuffer& rRenderBuffer, const TextureSampler& rSampler,
                       bool isDepth) const
{
    rRenderBuffer.bind(pDrawContext);
    {
        sead::Viewport viewport(rRenderBuffer);
        viewport.apply(pDrawContext, rRenderBuffer);
    }

    bool useGather = mFlags.isOn(1 << 30);
    const TextureData& rTextureData = rSampler.getTextureData();
    f32 width = rTextureData.getWidth(0);
    f32 height = rTextureData.getHeight(0);

    const detail::ShaderHolder* pHolder = detail::ShaderHolder::instance();

    if (level >= 1 && useGather)
    {
        const ShaderProgram* pBase =
            pHolder->getShaderProgramUnsafe(detail::ShaderHolder::cSsiiReduceG);
        const ShaderProgram* pProgram =
            pBase->getVariation(pBase->getVariationMacroStride(1) * level);
        pProgram->activate(pDrawContext, true);
        {
            sead::Vector2f step(1.0f / width, 1.0f / height);
            pProgram->getUniformLocation(0).setUniform(pDrawContext, 2, &step);
        }

        rSampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);
        pfx::detail::drawQuadTriangle(pDrawContext);
        rRenderBuffer.getRenderTargetColor()->invalidateGPUCache(pDrawContext);

        pProgram = pProgram->getVariation(pProgram->getVariationMacroStride(0) +
                                          pProgram->getVariationMacroStride(1) * level);
        pProgram->activate(pDrawContext, true);
        {
            sead::Vector2f step(1.0f / width, 1.0f / height);
            pProgram->getUniformLocation(0).setUniform(pDrawContext, 2, &step);
        }

        {
            TextureSampler sampler(
                *reinterpret_cast<const TextureData*>(rRenderBuffer.getRenderTargetColor()));
            sampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);
        }

        pfx::detail::drawQuadTriangle(pDrawContext);
        rRenderBuffer.getRenderTargetColor()->invalidateGPUCache(pDrawContext);
    }
    else
    {
        const ShaderProgram* pBase =
            pHolder->getShaderProgramUnsafe(detail::ShaderHolder::cSsiiReduce);
        const ShaderProgram* pProgram = pBase->getVariation(
            pBase->getVariationMacroStride(0) * level +
            (isDepth ? pBase->getVariationMacroStride(1) : 0));
        pProgram->activate(pDrawContext, true);
        {
            sead::Vector2f step(1.0f / width, 1.0f / height);
            pProgram->getUniformLocation(0).setUniform(pDrawContext, 2, &step);
        }

        rSampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);
        pfx::detail::drawQuadTriangle(pDrawContext);
        rRenderBuffer.getRenderTargetColor()->invalidateGPUCache(pDrawContext);
    }
}

/**
 * Combines light and albedo into a temporary buffer and downsamples it.
 * @param pDrawContext draw context
 * @param view view index
 * @param level reduction shader level
 * @param rRenderBuffer destination render buffer
 * @param rLight light sampler
 * @param rAlbedo albedo sampler
 */
void SSII::drawReduceWithPreRender_(DrawContext* pDrawContext, s32 view, s32 level,
                                    const RenderBuffer& rRenderBuffer,
                                    const TextureSampler& rLight,
                                    const TextureSampler& rAlbedo) const
{
    const TextureData& rTextureData =
        *reinterpret_cast<const TextureData*>(rRenderBuffer.getRenderTargetColor());
    u32 width = rTextureData.getWidth(0);
    u32 height = rTextureData.getHeight(0);

    TexBuf buffer;
    buffer.init(pDrawContext, cBufType_Albedo, width, height, "temp_pre_render_buf", false);
    buffer.mRenderBuffer.bind(pDrawContext);
    sead::Viewport viewport(buffer.mRenderBuffer);
    viewport.apply(pDrawContext, buffer.mRenderBuffer);

    const ShaderProgram* pProgram =
        detail::ShaderHolder::instance()->getShaderProgram(detail::ShaderHolder::cSsiiPreRender);
    pProgram->activate(pDrawContext, true);
    rLight.activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);
    rAlbedo.activate(pDrawContext, pProgram->getSamplerLocation(1), -1, false);
    pfx::detail::drawQuadTriangle(pDrawContext);
    buffer.mRenderBuffer.getRenderTargetColor()->invalidateGPUCache(pDrawContext);

    drawReduce_(pDrawContext, view, level, rRenderBuffer, buffer.mSampler, false);
}

/**
 * Gathers diffuse indirect light from one reduction level into another.
 * @param pDrawContext draw context
 * @param view view index
 * @param rSetting render setting of the source level
 * @param rRenderBuffer destination render buffer
 * @param rViewport destination viewport
 * @param rSrc source reduction buffer
 * @param rDst destination reduction buffer
 * @param useLight whether to use the light variation
 */
void SSII::drawSSII_(DrawContext* pDrawContext, s32 view, const RedBufRenderSetting& rSetting,
                     const RenderBuffer& rRenderBuffer, const sead::Viewport& rViewport,
                     const ReduceBuffer& rSrc, const ReduceBuffer& rDst, bool useLight) const
{
    const Context& rContext = mContexts[view];
    rRenderBuffer.bind(pDrawContext);
    rViewport.apply(pDrawContext, rRenderBuffer);

    const TextureData& rTextureData = *rSrc.mBuffers[cBufType_Light].mTextureData;
    f32 width = rTextureData.getWidth(0);
    f32 height = rTextureData.getHeight(0);
    f32 dstWidth = rViewport.getSizeX();
    f32 dstHeight = rViewport.getSizeY();

    const ShaderProgram* pBase =
        detail::ShaderHolder::instance()->getShaderProgramUnsafe(detail::ShaderHolder::cSsii);
    const ShaderProgram* pProgram =
        pBase->getVariation(pBase->getVariationMacroStride(0) * useLight +
                            rSetting.mSampleNum * pBase->getVariationMacroStride(1) +
                            rSetting.mMode * pBase->getVariationMacroStride(2));
    pProgram->activate(pDrawContext, true);
    pProgram->getUniformLocation(0).setUniform(pDrawContext, 2, &rContext.mTanFovyHalf);
    pProgram->getUniformLocation(1).setUniform(pDrawContext, 2, &rContext.mProjOffset);
    setUniformVec2(pProgram->getUniformLocation(2), pDrawContext,
                   sead::Vector2f(1.0f / width, 1.0f / height));
    setUniformVec2(pProgram->getUniformLocation(3), pDrawContext,
                   sead::Vector2f(1.0f / dstWidth, 1.0f / dstHeight));
    setUniformF32(pDrawContext, rSetting.mIntensity, pProgram->getUniformLocation(4));
    setUniformF32(pDrawContext, rContext.mViewFrustum.getFar() - rContext.mViewFrustum.getNear(), pProgram->getUniformLocation(5));
    setUniformF32(pDrawContext, rContext.mViewFrustum.getNear(), pProgram->getUniformLocation(6));
    setUniformF32(pDrawContext, mSteep, pProgram->getUniformLocation(7));

    rSrc.mBuffers[cBufType_Light].mSampler.activate(pDrawContext, pProgram->getSamplerLocation(0),
                                                     -1, false);
    rSrc.mBuffers[cBufType_Albedo].mSampler.activate(pDrawContext,
                                                      pProgram->getSamplerLocation(1), -1, false);
    rSrc.mBuffers[cBufType_Normal].mSampler.activate(pDrawContext,
                                                      pProgram->getSamplerLocation(3), -1, false);
    rSrc.mBuffers[cBufType_Depth].mSampler.activate(pDrawContext, pProgram->getSamplerLocation(2),
                                                     -1, false);
    rDst.mBuffers[cBufType_Albedo].mSampler.activate(pDrawContext,
                                                      pProgram->getSamplerLocation(5), -1, false);
    rDst.mBuffers[cBufType_Normal].mSampler.activate(pDrawContext,
                                                      pProgram->getSamplerLocation(7), -1, false);
    rDst.mBuffers[cBufType_Depth].mSampler.activate(pDrawContext, pProgram->getSamplerLocation(6),
                                                     -1, false);
    pfx::detail::drawQuadTriangle(pDrawContext);
    rRenderBuffer.getRenderTargetColor()->invalidateGPUCache(pDrawContext);
}

/**
 * Gathers reflected indirect light from one reduction level.
 * @param pDrawContext draw context
 * @param view view index
 * @param intensity reflection intensity
 * @param level shader level
 * @param mode shader mode
 * @param rRenderBuffer destination render buffer
 * @param rViewport destination viewport
 * @param rLight reduction buffer providing the light
 * @param rSrc source reduction buffer
 * @param rDst destination reduction buffer
 */
void SSII::drawRef_(DrawContext* pDrawContext, s32 view, f32 intensity, s32 level, s32 mode,
                    const RenderBuffer& rRenderBuffer, const sead::Viewport& rViewport,
                    const ReduceBuffer& rLight, const ReduceBuffer& rSrc,
                    const ReduceBuffer& rDst) const
{
    const Context& rContext = mContexts[view];
    rRenderBuffer.bind(pDrawContext);
    rViewport.apply(pDrawContext, rRenderBuffer);

    const TextureData& rTextureData = *rLight.mBuffers[cBufType_Light].mTextureData;
    f32 width = rTextureData.getWidth(0);
    f32 height = rTextureData.getHeight(0);
    f32 dstWidth = rViewport.getSizeX();
    f32 dstHeight = rViewport.getSizeY();

    const ShaderProgram* pBase =
        detail::ShaderHolder::instance()->getShaderProgramUnsafe(detail::ShaderHolder::cSsiiRef);
    const ShaderProgram* pProgram =
        pBase->getVariation(pBase->getVariationMacroStride(0) * level +
                            pBase->getVariationMacroStride(1) * mode);
    pProgram->activate(pDrawContext, true);
    pProgram->getUniformLocation(0).setUniform(pDrawContext, 2, &rContext.mTanFovyHalf);
    pProgram->getUniformLocation(1).setUniform(pDrawContext, 2, &rContext.mProjOffset);
    setUniformVec2(pProgram->getUniformLocation(2), pDrawContext,
                   sead::Vector2f(1.0f / width, 1.0f / height));
    setUniformVec2(pProgram->getUniformLocation(3), pDrawContext,
                   sead::Vector2f(1.0f / dstWidth, 1.0f / dstHeight));
    setUniformF32(pDrawContext, intensity, pProgram->getUniformLocation(4));
    setUniformF32(pDrawContext, rContext.mViewFrustum.getFar() - rContext.mViewFrustum.getNear(), pProgram->getUniformLocation(5));
    setUniformF32(pDrawContext, rContext.mViewFrustum.getNear(), pProgram->getUniformLocation(6));
    setUniformF32(pDrawContext, mSteep, pProgram->getUniformLocation(7));
    setUniformF32(pDrawContext, mRefPow, pProgram->getUniformLocation(8));
    setUniformF32(pDrawContext, mRefInflate, pProgram->getUniformLocation(9));

    rLight.mBuffers[cBufType_Light].mSampler.activate(pDrawContext,
                                                       pProgram->getSamplerLocation(0), -1, false);
    rSrc.mBuffers[cBufType_Albedo].mSampler.activate(pDrawContext,
                                                      pProgram->getSamplerLocation(1), -1, false);
    rSrc.mBuffers[cBufType_Normal].mSampler.activate(pDrawContext,
                                                      pProgram->getSamplerLocation(3), -1, false);
    rSrc.mBuffers[cBufType_Depth].mSampler.activate(pDrawContext, pProgram->getSamplerLocation(2),
                                                     -1, false);
    rDst.mBuffers[cBufType_Albedo].mSampler.activate(pDrawContext,
                                                      pProgram->getSamplerLocation(5), -1, false);
    rDst.mBuffers[cBufType_Normal].mSampler.activate(pDrawContext,
                                                      pProgram->getSamplerLocation(7), -1, false);
    rDst.mBuffers[cBufType_Depth].mSampler.activate(pDrawContext, pProgram->getSamplerLocation(6),
                                                     -1, false);
    pfx::detail::drawQuadTriangle(pDrawContext);
    rRenderBuffer.getRenderTargetColor()->invalidateGPUCache(pDrawContext);
}

/**
 * Suppresses feedback between frames by comparing against the average brightness.
 * @param pDrawContext draw context
 * @param view view index
 * @param rRenderBuffer destination render buffer
 * @param rSrc0 current result sampler
 * @param rSrc1 average brightness sampler
 */
void SSII::drawAntiHowling_(DrawContext* pDrawContext, s32 view,
                            const RenderBuffer& rRenderBuffer, const TextureSampler& rSrc0,
                            const TextureSampler& rSrc1) const
{
    rRenderBuffer.bind(pDrawContext);
    {
        sead::Viewport viewport(rRenderBuffer);
        viewport.apply(pDrawContext, rRenderBuffer);
    }

    const ShaderProgram* pProgram = detail::ShaderHolder::instance()->getShaderProgram(
        detail::ShaderHolder::cSsiiAntiHowling);
    pProgram->activate(pDrawContext, true);
    setUniformF32(pDrawContext, mAntiHowlingThreshold, pProgram->getUniformLocation(0));
    rSrc0.activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);
    rSrc1.activate(pDrawContext, pProgram->getSamplerLocation(1), -1, false);
    pfx::detail::drawQuadTriangle(pDrawContext);
    rRenderBuffer.getRenderTargetColor()->invalidateGPUCache(pDrawContext);
}

void SSII::drawExpand_(DrawContext* pDrawContext, s32 view, const RenderBuffer& rRenderBuffer,
                       const TextureSampler& rSrcLight, const TextureSampler& rSrcNormal,
                       const TextureSampler& rSrcDepth, const TextureSampler& rDstNormal,
                       const TextureSampler& rDstDepth, const TextureSampler* pDstAlbedo,
                       const TextureSampler* pSrcLight1, s32 level, bool isFinal) const
{
    const Context& rContext = mContexts[view];
    bool isMRT = pSrcLight1 != nullptr && pDstAlbedo == nullptr;
    rRenderBuffer.bind(pDrawContext);
    {
        sead::Viewport viewport(rRenderBuffer);
        viewport.apply(pDrawContext, rRenderBuffer);
    }

    const TextureData& rSrcTexture = rSrcLight.getTextureData();
    f32 srcWidth = rSrcTexture.getWidth(0);
    f32 srcHeight = rSrcTexture.getHeight(0);
    const TextureData& rDstTexture =
        *reinterpret_cast<const TextureData*>(rRenderBuffer.getRenderTargetColor());
    f32 dstWidth = rDstTexture.getWidth(0);
    f32 dstHeight = rDstTexture.getHeight(0);

    const ShaderProgram* pBase =
        detail::ShaderHolder::instance()->getShaderProgramUnsafe(detail::ShaderHolder::cSsiiExpand);
    const ShaderProgram* pProgram = pBase->getVariation(
        (pSrcLight1 != nullptr ? pBase->getVariationMacroStride(0) : 0) +
        pBase->getVariationMacroStride(1) * level +
        pBase->getVariationMacroStride(2) * mFlags.isOn(1 << 30) +
        (isFinal ? pBase->getVariationMacroStride(3) * 2 :
                   (pDstAlbedo != nullptr ? pBase->getVariationMacroStride(3) : 0)));
    pProgram->activate(pDrawContext, true);
    pProgram->getUniformLocation(0).setUniform(pDrawContext, 2, &rContext.mTanFovyHalf);
    pProgram->getUniformLocation(1).setUniform(pDrawContext, 2, &rContext.mProjOffset);
    setUniformVec2(pProgram->getUniformLocation(2), pDrawContext,
                   sead::Vector2f(1.0f / srcWidth, 1.0f / srcHeight));
    setUniformVec2(pProgram->getUniformLocation(3), pDrawContext,
                   sead::Vector2f(1.0f / dstWidth, 1.0f / dstHeight));
    setUniformF32(pDrawContext, rContext.mViewFrustum.getFar() - rContext.mViewFrustum.getNear(), pProgram->getUniformLocation(4));
    setUniformF32(pDrawContext, rContext.mViewFrustum.getNear(), pProgram->getUniformLocation(5));

    rSrcLight.activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);
    rSrcNormal.activate(pDrawContext, pProgram->getSamplerLocation(2), -1, false);
    rSrcDepth.activate(pDrawContext, pProgram->getSamplerLocation(3), -1, false);
    rDstNormal.activate(pDrawContext, pProgram->getSamplerLocation(5), -1, false);
    rDstDepth.activate(pDrawContext, pProgram->getSamplerLocation(6), -1, false);

    if (pSrcLight1 != nullptr)
    {
        pSrcLight1->activate(pDrawContext, pProgram->getSamplerLocation(1), -1, false);
    }

    if (pDstAlbedo != nullptr)
    {
        pDstAlbedo->activate(pDrawContext, pProgram->getSamplerLocation(4), -1, false);
    }

    pfx::detail::drawQuadTriangle(pDrawContext);
    rRenderBuffer.getRenderTargetColor()->invalidateGPUCache(pDrawContext);

    if (isMRT)
    {
        rRenderBuffer.getRenderTargetColor(1)->invalidateGPUCache(pDrawContext);
    }
}

/**
 * Copies a texture into the reprojection buffer of a view.
 * @param pDrawContext draw context
 * @param view view index
 * @param rTextureData texture to copy
 */
void SSII::setReprojectionBuffer(DrawContext* pDrawContext, s32 view,
                                 const TextureData& rTextureData) const
{
    if (!mFlags.isOnAll(1 | 1 << 6))
    {
        return;
    }

    const Context& rContext = mContexts[view];
    const TexBuf& rBuffer = rContext.mReprojectionBuffer;
    f32 width = rBuffer.mWidth;
    f32 height = rBuffer.mHeight;
    mGraphicsContext[0].apply(pDrawContext);
    rBuffer.mRenderBuffer.bind(pDrawContext);
    rBuffer.mViewport.apply(pDrawContext, rBuffer.mRenderBuffer);

    const ShaderProgram* pProgram = detail::ShaderHolder::instance()
                                        ->getShaderProgramUnsafe(detail::ShaderHolder::cSsiiReduce)
                                        ->getVariation(0);
    pProgram->activate(pDrawContext, true);
    TextureSampler sampler(rTextureData);
    sead::Vector2f step(1.0f / width, 1.0f / height);
    pProgram->getUniformLocation(0).setUniform(pDrawContext, 2, &step);
    sampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);
    pfx::detail::drawQuadTriangle(pDrawContext);
    rBuffer.mRenderBuffer.getRenderTargetColor()->invalidateGPUCache(pDrawContext);
}

/**
 * Does nothing.
 * @param pDrawContext draw context
 * @param view view index
 */
void SSII::drawDebug(DrawContext* pDrawContext, s32 view) const {}

/**
 * Generates the host IO messages of the debug pages and parameters.
 * @param pContext host IO context
 */
void SSII::genMessage(sead::hostio::Context* pContext)
{
    mDebugPage.genMessagePage(pContext, this);
    {
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d", 0, 10);
    }

    {
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d", 0, 10);
    }

    {
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d", 0, 10);
    }

    {
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d", 0, 1);
    }

    {
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d, Mode = MinMaxLock", 0, 4);
    }

    {
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d, Mode = MinMaxLock", 0, 4);
    }

    {
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d, Mode = MinMaxLock", 0, 4);
    }

    {
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d, Mode = MinMaxLock", 0, 4);
    }

    {
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d, Mode = MinMaxLock", 0, 4);
    }

    {
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d, Mode = MinMaxLock", 1, 10);
    }

    {
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d, Mode = MinMaxLock", 1, 10);
    }

    {
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d, Mode = MinMaxLock", 0, 10);
    }

    {
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d, Mode = MinMaxLock", 0, 10);
    }

    mDifDebugPage.genMessagePage(pContext, this);
    mRefDebugPage.genMessagePage(pContext, this);

    {
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d", 0, 10);
    }

    {
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d", 0, 10);
    }

    {
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d", 0, 2);
    }

    {
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d", -1, 1);
    }

    {
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d, Mode = MinMaxLock", 0, 4);
    }

    {
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d, Mode = MinMaxLock", 0, 4);
    }

    {
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d, Mode = MinMaxLock", 0, 4);
    }

    for (s32 i = 0; i < 10; i++)
    {
        sead::FormatFixedSafeString<256> label("Start %d", i);
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d, Mode = MinMaxLock", 0, 4);
    }

    for (s32 i = 0; i < 10; i++)
    {
        sead::FormatFixedSafeString<256> label("Num %d", i);
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d, Mode = MinMaxLock", 0, 4);
    }

    for (s32 i = 0; i < 10; i++)
    {
        sead::FormatFixedSafeString<256> label("Intensity %d", i);
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d", 0, 1);
    }

    for (s32 i = 0; i < 10; i++)
    {
        sead::FormatFixedSafeString<256> label("%d", i);
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d, Mode = MinMaxLock", 0, 3);
    }

    {
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d", 0, 10);
    }

    {
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d", -1, 1);
    }

    {
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d", 0, 100);
    }

    {
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d", 0, 2);
    }

    {
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d, Mode = MinMaxLock", 0, 4);
    }

    {
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d, Mode = MinMaxLock", 0, 4);
    }

    {
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d, Mode = MinMaxLock", 0, 4);
    }

    for (s32 i = 0; i < 10; i++)
    {
        sead::FormatFixedSafeString<256> label("Start %d", i);
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d, Mode = MinMaxLock", 0, 4);
    }

    for (s32 i = 0; i < 10; i++)
    {
        sead::FormatFixedSafeString<256> label("Num %d", i);
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d, Mode = MinMaxLock", 0, 4);
    }

    for (s32 i = 0; i < 10; i++)
    {
        sead::FormatFixedSafeString<256> label("Intensity %d", i);
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d", 0, 1);
    }

    for (s32 i = 0; i < 10; i++)
    {
        sead::FormatFixedSafeString<256> label("%d", i);
        sead::FormatFixedSafeString<64> meta("Min = %d, Max = %d, Mode = MinMaxLock", 0, 3);
    }
}

/**
 * Reapplies the quality settings when one of them is changed.
 * @param pEvent property event
 */
void SSII::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    if (pEvent->getType() & 2)
    {
        return;
    }

    const void* pId = pEvent->getId();

    if ((pId < &mRedBufQuality + 1 && pId >= &mRedBufQuality) ||
        (pId < &mSampleQuality + 1 && pId >= &mSampleQuality) ||
        (pId < &mDifQuality + 1 && pId >= &mDifQuality) ||
        (pId < &mRefQuality + 1 && pId >= &mRefQuality) ||
        (pId < &mRefStartOffset + 1 && pId >= &mRefStartOffset))
    {
        applyQualitySetting_();
    }
}

}  // namespace agl::lght

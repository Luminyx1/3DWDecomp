#include "shadow/aglShadowMap.h"

#include <gfx/seadGraphicsContext.h>
#include <gfx/seadViewport.h>
#include <hostio/seadHostIOPropertyEvent.h>
#include <math/seadMathCalcCommon.h>

#include "common/aglDrawContext.h"
#include "common/aglTextureFormatInfo.h"
#include "utility/aglDevTools.h"
#include "utility/aglDynamicTextureAllocator.h"
#include "utility/aglImageFilter2D.h"

namespace agl::sdw
{

/**
 * Constructs the shadow map with its default parameters.
 */
ShadowMap::ShadowMap()
    : mShadowMapType(0, "shadow_map_type", "Shadow Map Type", this),
      mSizeW(0x200, "size_w", "Texture Width", "Min=32, Max=2048", this),
      mSizeH(0x200, "size_h", "Texture Height", "Min=32, Max=2048", this),
      mReduceType(0, "reduce_type", "Reduction Buffer Generation Type", this),
      mEnableHiZ(false, "enable_hiz", "Enable Hi Z", this),
      mForceArray(false, "force_array", "Force Array", this),
      mUse16UNorm(false, "use_16_unorm", "16_UNorm", this),
      mAllocWithoutContext(false, "alloc_without_ctx", "WithoutContext", this),
      mAllocFromMem1(true, "alloc_from_mem1", "Mem1", this),
      mExpandToMem2(false, "expand_to_mem2", "Expand To Mem2", this),
      mExpandAllSlice(true, "expand_all_slice", "Exapand all slices", this),
      mScissor(true, "scissor", "Scissor", this),
      mCreateHalf(false, "create_half", "Enable Half Cascades", this),
      mHalfCascadeNum(-1, "half_size_cascade_num", "Cascade Count (Half)", "Min=-1, Max=4", this),
      mCreateQuarter(false, "create_quarter", "Enable Quarter CAscades", this),
      mQuarterCascadeNum(-1, "quater_size_cascade_num", "Cascade Count (Quarter)", "Min=-1, Max=4",
                         this),
      mReduceMem1(true, "reduce_mem1", "Use Mem1 for reduction buffer", this),
      mScissorMargin(1.0f, "scissor_margin", "Scissor Margin", "Min=0,  Max=4", this)
{
}

/**
 * Releases every texture and the debug page.
 */
ShadowMap::~ShadowMap()
{
    free();
    mDebugTexturePage.cleanUp();
}

/**
 * Releases every allocated texture.
 */
void ShadowMap::free()
{
    freeTexture_(mDepthTexture);
    freeTexture_(mVarianceTexture);
    freeTexture_(mExpandTexture);
    freeTexture_(mHalfTexture);
    freeTexture_(mQuarterTexture);
}

/**
 * Initializes the samplers and the debug texture page.
 * @param rArg creation arguments (unused)
 * @param pHeap heap for the debug texture page
 */
void ShadowMap::initialize(const CreateArg& rArg, sead::Heap* pHeap)
{
    initializeDepthSampler_(&mDepthTextureSampler);
    initializeDepthSampler_(&mHalfSampler);
    initializeDepthSampler_(&mQuarterSampler);
    mDepthSampler = &mDepthTextureSampler;
    mDebugTexturePage.setUp(1, "Shadow Map", pHeap);
}

/**
 * Configures a sampler for depth comparison lookups.
 * @param pSampler sampler to configure
 */
void ShadowMap::initializeDepthSampler_(TextureSampler* pSampler)
{
    pSampler->setWrap(5, 5, 7);
    pSampler->setBorderColorAsColor(sead::Color4f::cWhite);
    pSampler->setDepthCompareEnable(true);
    pSampler->setDepthCompareFunc(4);
}

/**
 * Allocates the depth texture (and the expanded copy) and binds it to the render target.
 * @param pDrawContext draw context
 * @param sliceNum number of cascades
 * @param mipLevelNum number of mip levels
 */
void ShadowMap::allocDepthBuffer(DrawContext* pDrawContext, s32 sliceNum, s32 mipLevelNum)
{
    utl::DynamicTextureAllocator* allocator = utl::DynamicTextureAllocator::instance();
    GPUMemVoidAddr hiZAddr;
    const TextureFormat format = *mUse16UNorm ? TextureFormat::cTextureFormat_Depth_16 :
                                                TextureFormat::cTextureFormat_Depth_32;
    const TextureFormat expandFormat = *mUse16UNorm ? TextureFormat::cTextureFormat_R16_uNorm :
                                                      TextureFormat::cTextureFormat_R32_float;
    const auto type = static_cast<utl::DynamicTextureAllocator::AllocateType>(!*mAllocFromMem1);

    if (sliceNum != 1 || *mForceArray)
    {
        if (*mAllocWithoutContext)
        {
            mDepthTexture = allocator->allocArrayWithoutContext(
                pDrawContext, "shadow_map_depth", format, *mSizeW, *mSizeH, sliceNum, mipLevelNum,
                *mEnableHiZ ? &hiZAddr : nullptr, type, true, false);
        }
        else
        {
            mDepthTexture = allocator->allocArray(
                pDrawContext, "shadow_map_depth", format, *mSizeW, *mSizeH, sliceNum, mipLevelNum,
                *mEnableHiZ ? &hiZAddr : nullptr, type, true, false);
        }

        if (*mExpandToMem2 && *mEnableHiZ)
        {
            if (*mAllocWithoutContext)
            {
                mExpandTexture = allocator->allocArrayWithoutContext(
                    pDrawContext, "shadow_map_expand_mwm2", expandFormat, *mSizeW, *mSizeH,
                    sliceNum, mipLevelNum, nullptr, utl::DynamicTextureAllocator::cAllocateType_1,
                    true, false);
            }
            else
            {
                mExpandTexture = allocator->allocArray(
                    pDrawContext, "shadow_map_expand_mem2", expandFormat, *mSizeW, *mSizeH,
                    sliceNum, mipLevelNum, nullptr, utl::DynamicTextureAllocator::cAllocateType_1,
                    true, false);
            }
        }
    }
    else
    {
        if (*mAllocWithoutContext)
        {
            mDepthTexture = allocator->allocWithoutContext(
                pDrawContext, "shadow_map_depth", format, *mSizeW, *mSizeH, mipLevelNum,
                *mEnableHiZ ? &hiZAddr : nullptr, type, true, false);
        }
        else
        {
            mDepthTexture =
                allocator->alloc(pDrawContext, "shadow_map_depth", format, *mSizeW, *mSizeH,
                                 mipLevelNum, *mEnableHiZ ? &hiZAddr : nullptr, type, true, false);
        }

        if (*mExpandToMem2 && *mEnableHiZ)
        {
            if (*mAllocWithoutContext)
            {
                mExpandTexture = allocator->allocWithoutContext(
                    pDrawContext, "shadow_map_expand_mem2", expandFormat, *mSizeW, *mSizeH,
                    mipLevelNum, nullptr, utl::DynamicTextureAllocator::cAllocateType_1, true,
                    false);
            }
            else
            {
                mExpandTexture =
                    allocator->alloc(pDrawContext, "shadow_map_expand_mem2", expandFormat, *mSizeW,
                                     *mSizeH, mipLevelNum, nullptr,
                                     utl::DynamicTextureAllocator::cAllocateType_1, true, false);
            }
        }
    }

    mIsAllocWithoutContext = *mAllocWithoutContext;
    mRenderTargetDepth.applyTextureData(*mDepthTexture, 0, 0);
    mRenderTargetDepth.setZCullBuffer(*mEnableHiZ ? hiZAddr : GPUMemVoidAddr(GPUMemAddrBase()));
    if (*mExpandToMem2 && *mEnableHiZ)
    {
        mDepthTextureSampler.applyTextureData(*mExpandTexture);
    }
    else
    {
        mDepthTextureSampler.applyTextureData(*mDepthTexture);
    }
}

/**
 * Releases the full resolution textures.
 */
void ShadowMap::freeFullOnly()
{
    freeTexture_(mDepthTexture);
    freeTexture_(mVarianceTexture);
    freeTexture_(mExpandTexture);
}

/**
 * Releases the half resolution texture.
 */
void ShadowMap::freeHalf() const
{
    freeTexture_(mHalfTexture);
}

/**
 * Releases the quarter resolution texture.
 */
void ShadowMap::freeQuat() const
{
    freeTexture_(mQuarterTexture);
}

/**
 * Releases the full resolution depth texture.
 */
void ShadowMap::freeFull_() const
{
    freeTexture_(mDepthTexture);
}

/**
 * Releases the depth texture and the reduced textures.
 */
void ShadowMap::freeDepth_() const
{
    freeTexture_(mDepthTexture);
    freeTexture_(mHalfTexture);
    freeTexture_(mQuarterTexture);
}

/**
 * Binds and clears one cascade slice of the depth buffer.
 * @param pDrawContext draw context
 * @param index cascade index
 * @param rOffset cascade offset in texture space
 * @param rScale cascade scale in texture space
 */
void ShadowMap::beginDepthBuffer(DrawContext* pDrawContext, s32 index,
                                 const sead::Vector2f& rOffset, const sead::Vector2f& rScale)
{
    const s32 sliceNum = mRenderTargetDepth.getMipSlice(0);
    if (mIsDirty)
    {
        const s32 mipLevelNum = mRenderTargetDepth.getMipLevelNum();
        free();
        allocDepthBuffer(pDrawContext, sliceNum, mipLevelNum);
        mIsDirty = false;
    }

    mRenderTargetDepth.setSlice(index);
    mRenderBuffer.setRenderTargetColorNullAll();

    const sead::Vector2f size(*mSizeW, *mSizeH);
    mRenderBuffer.setVirtualSize(size);
    mRenderBuffer.setPhysicalArea(0.0f, 0.0f, static_cast<f32>(*mSizeW), static_cast<f32>(*mSizeH));
    mRenderBuffer.setRenderTargetDepth(&mRenderTargetDepth);
    mRenderBuffer.bind(pDrawContext);

    {
        sead::Viewport viewport(mRenderBuffer);
        viewport.apply(pDrawContext, mRenderBuffer);
    }

    {
        sead::Viewport viewport(mRenderBuffer);
        mRenderBuffer.fastClear(pDrawContext, 0, 2, sead::Color4f::cWhite, 1.0f, 0, viewport, true);
    }

    if (*mScissor)
    {
        const f32 w = size.x;
        const f32 h = size.y;
        const f32 margin = *mScissorMargin;
        const f32 left = rOffset.x * w - margin;
        const f32 top = rOffset.y * h - margin;
        const f32 margin2 = margin * 2.0f;
        const f32 right = margin2 + rScale.x * w;
        const f32 bottom = margin2 + rScale.y * h;
        sead::Viewport viewport(
            sead::Mathf::clampMin(static_cast<f32>(sead::Mathf::floor(left)), 0.0f),
            sead::Mathf::clampMin(static_cast<f32>(sead::Mathf::floor(top)), 0.0f),
            sead::Mathf::min(w, static_cast<f32>(sead::Mathf::ceil(right))),
            sead::Mathf::min(h, static_cast<f32>(sead::Mathf::ceil(bottom))));
        viewport.applyScissor(pDrawContext, mRenderBuffer);
    }
}

/**
 * Finishes rendering one cascade slice and expands the Hi-Z buffer when needed.
 * @param pDrawContext draw context
 * @param index cascade index
 */
void ShadowMap::endDepthBuffer(DrawContext* pDrawContext, s32 index)
{
    if (!*mExpandAllSlice && *mEnableHiZ && !*mExpandToMem2)
    {
        mRenderTargetDepth.expandHiZBuffer(pDrawContext);
    }

    mRenderTargetDepth.invalidateGPUCache(pDrawContext);

    if (mDepthTextureSampler.getTextureData().getMipSlice(0) - 1 != index)
    {
        return;
    }

    if (*mExpandAllSlice)
    {
        if (*mEnableHiZ)
        {
            if (*mExpandToMem2)
            {
                freeDepth_();
            }
            else if (index != 0)
            {
                mRenderTargetDepth.expandHiZBufferAllSlice(pDrawContext);
            }
            else
            {
                mRenderTargetDepth.expandHiZBuffer(pDrawContext);
            }
        }
    }
    else if (*mExpandToMem2 && *mEnableHiZ)
    {
        freeDepth_();
    }

    mDepthSampler = &mDepthTextureSampler;
}

/**
 * Generates the half and quarter resolution reduction buffers.
 * @param pDrawContext draw context
 */
void ShadowMap::drawReduce(DrawContext* pDrawContext) const
{
    if (!*mCreateHalf && !*mCreateQuarter)
    {
        return;
    }

    const TextureData& depthTexture = mDepthTextureSampler.getTextureData();
    auto format = static_cast<TextureFormat>(depthTexture.getTextureFormat());
    const auto type = static_cast<utl::DynamicTextureAllocator::AllocateType>(!*mReduceMem1);
    switch (format)
    {
    case TextureFormat::cTextureFormat_Depth_16:
        format = TextureFormat::cTextureFormat_R16_uNorm;
        break;
    case TextureFormat::cTextureFormat_Depth_32:
        format = TextureFormat::cTextureFormat_R32_float;
        break;
    default:
        break;
    }

    const bool isDepth = TextureFormatInfo::isUsableAsRenderTargetDepth(format);
    s32 height = *mSizeH;
    s32 width = *mSizeW;
    const u16 textureType = depthTexture.getTextureType();

    RenderTargetDepth* pDepthTarget;
    if (isDepth)
    {
        sead::GraphicsContext context;
        context.setDepthEnable(true, true);
        context.setColorMask(false, false, false, false);
        context.setDepthFunc(8);
        context.setBlendEnable(false);
        context.apply(pDrawContext);
        mRenderBuffer.setRenderTargetColorNullAll();
        pDepthTarget = &mReduceTargetDepth;
    }
    else
    {
        sead::GraphicsContext context;
        context.setColorMask(true, false, false, false);
        context.setDepthEnable(false, false);
        context.setBlendEnable(false);
        context.apply(pDrawContext);
        mRenderBuffer.setRenderTargetColor(&mRenderTargetColor);
        pDepthTarget = nullptr;
    }

    mRenderBuffer.setRenderTargetDepth(pDepthTarget);

    mReduceSampler.applyTextureData(depthTexture);
    if (*mReduceType == 3)
    {
        mReduceSampler.setFilter(0, 0, 0);
    }
    else
    {
        mReduceSampler.setFilter(1, 1, 0);
    }

    for (s32 i = 0; i < 2; i++)
    {
        width /= 2;
        height /= 2;
        mRenderBuffer.setVirtualSize(sead::Vector2f(width, height));
        mRenderBuffer.setPhysicalArea(0.0f, 0.0f, static_cast<f32>(width),
                                      static_cast<f32>(height));
        {
            sead::Viewport viewport(mRenderBuffer);
            viewport.apply(pDrawContext, mRenderBuffer);
        }

        const s32 maxSliceNum = depthTexture.getMipSlice(0);
        const s32 cascadeNum = i == 0 ? *mHalfCascadeNum : *mQuarterCascadeNum;
        s32 sliceNum = cascadeNum == -1 ? maxSliceNum : cascadeNum;
        sliceNum = sead::Mathi::min(sliceNum, depthTexture.getMipSlice(0));

        utl::DynamicTextureAllocator* allocator = utl::DynamicTextureAllocator::instance();
        const TextureData* pTexture;
        if (textureType == NVN_TEXTURE_TARGET_2D_ARRAY)
        {
            if (*mAllocWithoutContext)
            {
                pTexture = allocator->allocArrayWithoutContext(
                    pDrawContext, "shadow_map_reduce_depth", format, width, height, sliceNum, 1,
                    nullptr, type, true, false);
            }
            else
            {
                pTexture =
                    allocator->allocArray(pDrawContext, "shadow_map_reduce_depth", format, width,
                                          height, sliceNum, 1, nullptr, type, true, false);
            }

            for (s32 slice = 0; slice < sliceNum; slice++)
            {
                if (isDepth)
                {
                    mReduceTargetDepth.applyTextureData(*pTexture, 0, slice);
                }
                else
                {
                    mRenderTargetColor.applyTextureData(*pTexture, 0, slice);
                }

                mRenderBuffer.bind(pDrawContext);

                switch (*mReduceType)
                {
                case 0:
                    utl::ImageFilter2D::draw2DArrayMinMaxQuadTriangle(pDrawContext, mReduceSampler,
                                                                      false, isDepth, slice);
                    break;
                case 1:
                    utl::ImageFilter2D::draw2DArrayMinMaxQuadTriangle(pDrawContext, mReduceSampler,
                                                                      true, isDepth, slice);
                    break;
                case 2:
                case 3:
                    if (isDepth)
                    {
                        utl::ImageFilter2D::draw2DArrayDepthQuadTriangle(pDrawContext,
                                                                         mReduceSampler, slice);
                    }
                    else
                    {
                        utl::ImageFilter2D::draw2DArrayColorQuadTriangle(pDrawContext,
                                                                         mReduceSampler, slice);
                    }

                    break;
                default:
                    break;
                }
            }
        }
        else
        {
            if (*mAllocWithoutContext)
            {
                pTexture =
                    allocator->allocWithoutContext(pDrawContext, "shadow_map_reduce_depth", format,
                                                   width, height, 1, nullptr, type, true, false);
            }
            else
            {
                pTexture = allocator->alloc(pDrawContext, "shadow_map_reduce_depth", format, width,
                                            height, 1, nullptr, type, true, false);
            }

            if (isDepth)
            {
                mReduceTargetDepth.applyTextureData(*pTexture, 0, 0);
            }
            else
            {
                mRenderTargetColor.applyTextureData(*pTexture, 0, 0);
            }

            mRenderBuffer.bind(pDrawContext);

            switch (*mReduceType)
            {
            case 0:
                utl::ImageFilter2D::draw2DMinMaxQuadTriangle(pDrawContext, mReduceSampler, false,
                                                             isDepth);
                break;
            case 1:
                utl::ImageFilter2D::draw2DMinMaxQuadTriangle(pDrawContext, mReduceSampler, true,
                                                             isDepth);
                break;
            case 2:
            case 3:
                if (isDepth)
                {
                    utl::ImageFilter2D::drawDepthQuadTriangle(pDrawContext, mReduceSampler);
                }
                else
                {
                    utl::ImageFilter2D::drawTextureQuadTriangle(pDrawContext, mReduceSampler);
                }

                break;
            default:
                break;
            }
        }

        if (i != 0)
        {
            mQuarterTexture = pTexture;
            mQuarterSampler.applyTextureData(*pTexture);
        }
        else
        {
            mHalfTexture = pTexture;
            mHalfSampler.applyTextureData(*pTexture);
        }

        mReduceSampler.applyTextureData(*pTexture);

        if (!*mCreateQuarter)
        {
            break;
        }
    }

    if (!*mCreateHalf)
    {
        utl::DynamicTextureAllocator::instance()->free(mHalfTexture);
        mHalfTexture = nullptr;
    }
}

/**
 * Sets the shadow map size, rounded up to a multiple of 8.
 * @param rSize requested size
 */
void ShadowMap::setSize(const sead::Vector2i& rSize)
{
    *mSizeW = (rSize.x + 7) & ~7;
    *mSizeH = (rSize.y + 7) & ~7;
}

/**
 * Draws a visualization of one cascade of the shadow map.
 * @param pDrawContext draw context
 * @param index cascade index
 * @param rTexMtx texture matrix of the cascade
 * @param rViewMtx camera view matrix
 * @param rProjMtx camera projection matrix
 */
void ShadowMap::drawDebug(DrawContext* pDrawContext, s32 index, const sead::Matrix44f& rTexMtx,
                          const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx)
{
    utl::DevTools::drawVisualizedDepth(pDrawContext, mDepthSampler->getTextureData(), index,
                                       rTexMtx, rViewMtx, rProjMtx);
}

/**
 * Generates the host IO message.
 * @param pContext host IO context
 */
void ShadowMap::genMessage(sead::hostio::Context* pContext)
{
    mDebugTexturePage.genMessagePage(pContext, this);
    genMessageDebugParameter(pContext, this);
}

/**
 * Generates the parameter host IO messages.
 * @param pContext host IO context
 * @param pNode parent node (unused)
 */
void ShadowMap::genMessageParameter(sead::hostio::Context* pContext, sead::hostio::Node* pNode)
{
    mDebugTexturePage.genMessagePage(pContext, pNode);
}

/**
 * Generates the debug parameter host IO messages.
 * @param pContext host IO context
 * @param pNode parent node (unused)
 */
void ShadowMap::genMessageDebugParameter(sead::hostio::Context* pContext, sead::hostio::Node* pNode)
{
    mSizeW.genMessageParameter(pContext, mSizeW.getMeta());
    mSizeH.genMessageParameter(pContext, mSizeH.getMeta());
    mEnableHiZ.genMessageParameter(pContext, mEnableHiZ.getMeta());
    mExpandToMem2.genMessageParameter(pContext, mExpandToMem2.getMeta());
    mExpandAllSlice.genMessageParameter(pContext, mExpandAllSlice.getMeta());
    mUse16UNorm.genMessageParameter(pContext, mUse16UNorm.getMeta());
    mAllocFromMem1.genMessageParameter(pContext, mAllocFromMem1.getMeta());
    mAllocWithoutContext.genMessageParameter(pContext, mAllocWithoutContext.getMeta());
    mForceArray.genMessageParameter(pContext, mForceArray.getMeta());
    mScissor.genMessageParameter(pContext, mScissor.getMeta());
    mCreateHalf.genMessageParameter(pContext, mCreateHalf.getMeta());
    mHalfCascadeNum.genMessageParameter(pContext, mHalfCascadeNum.getMeta());
    mCreateQuarter.genMessageParameter(pContext, mCreateQuarter.getMeta());
    mQuarterCascadeNum.genMessageParameter(pContext, mQuarterCascadeNum.getMeta());
    mReduceMem1.genMessageParameter(pContext, mReduceMem1.getMeta());
    mScissorMargin.genMessageParameter(pContext, mScissorMargin.getMeta());
}

/**
 * Handles a host IO property event.
 * @param pEvent property event
 */
void ShadowMap::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    if (reinterpret_cast<uintptr_t>(pEvent->getId()) == 15001)
    {
        mIsDirty = true;
    }

    listenPropertyEventDebugParameter(pEvent);
}

/**
 * Handles a parameter host IO property event.
 * @param pEvent property event (unused)
 */
void ShadowMap::listenPropertyEventParameter(const sead::hostio::PropertyEvent* pEvent) {}

/**
 * Handles a debug parameter host IO property event and marks the buffers dirty on resize.
 * @param pEvent property event
 */
void ShadowMap::listenPropertyEventDebugParameter(const sead::hostio::PropertyEvent* pEvent)
{
    if (pEvent->getType() & 2)
    {
        return;
    }

    const void* id = pEvent->getId();
    if ((id < &*mSizeW + 1 && id >= &*mSizeW) || (id < &*mSizeH + 1 && id >= &*mSizeH))
    {
        mIsDirty = true;
    }
}

}  // namespace agl::sdw

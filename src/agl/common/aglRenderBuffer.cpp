#include "common/aglRenderBuffer.h"

#include <gfx/seadViewport.h>
#include <math/seadMathCalcCommon.h>
#include <nvn/nvn_FuncPtrInline.h>

#include "common/aglDrawContext.h"
#include "common/aglRenderTarget.h"
#include "driver/aglNVNMgr.h"

namespace agl {

namespace {

bool initTextureDataFrom(const TextureData* pSource, TextureData* pTextureData)
{
    if (pSource->getTextureFormat() == 0)
    {
        return false;
    }

    pTextureData->initialize_(
        TextureType(pSource->getTextureType()), TextureFormat(pSource->getTextureFormat()),
        sead::Mathu::max(u32(pSource->getWidth()), 1u), sead::Mathi::max(pSource->getMinHeight_(), pSource->getHeight()),
        sead::Mathi::max(pSource->getMinSlice_(), pSource->getDepth()),
        pSource->getMipLevelNum(), TextureAttribute(pSource->getTextureAttribute()),
        MultiSampleType(pSource->getMultiSampleType()), true);
    return true;
}

void clearTargets(const RenderBuffer* pRenderBuffer, DrawContext* pDrawContext, u32 target,
                  u32 flags, const sead::Color4f& rColor, f32 depth, u32 stencil)
{
    if (flags & sead::FrameBuffer::cColor)
    {
        nvnCommandBufferClearColor(pDrawContext->getNvnCommandBuffer(), target, &rColor.r,
                                   NVN_CLEAR_COLOR_MASK_RGBA);
    }

    if ((flags & (sead::FrameBuffer::cDepth | sead::FrameBuffer::cStencil)) &&
        (pRenderBuffer->getRenderTargetDepth() != nullptr))
    {
        u32 stencilMask = (flags & sead::FrameBuffer::cStencil) ? 0xff : 0;
        nvnCommandBufferClearDepthStencil(pDrawContext->getNvnCommandBuffer(), depth,
                                          (flags >> 1) & 1, stencil, stencilMask);
    }
}

}  // namespace

/**
 * Constructs a render buffer with a 1x1 area and no render targets.
 */
RenderBuffer::RenderBuffer()
    : sead::FrameBuffer(sead::Vector2f(1.0f, 1.0f), sead::BoundBox2f(0.0f, 0.0f, 1.0f, 1.0f))
{
    initialize_();
}

/**
 * Removes all render targets.
 */
void RenderBuffer::initialize_()
{
    setRenderTargetColorNullAll();
    mRenderTargetDepth = nullptr;
}

/**
 * Constructs a render buffer without render targets.
 * @param rVirtualSize virtual size
 * @param rPhysicalArea physical area
 */
RenderBuffer::RenderBuffer(const sead::Vector2f& rVirtualSize,
                           const sead::BoundBox2f& rPhysicalArea)
    : sead::FrameBuffer(rVirtualSize, rPhysicalArea)
{
    initialize_();
}

/**
 * Constructs a render buffer without render targets.
 * @param rVirtualSize virtual size
 * @param physicalX physical area left
 * @param physicalY physical area top
 * @param physicalW physical area width
 * @param physicalH physical area height
 */
RenderBuffer::RenderBuffer(const sead::Vector2f& rVirtualSize, f32 physicalX, f32 physicalY,
                           f32 physicalW, f32 physicalH)
    : sead::FrameBuffer(rVirtualSize, physicalX, physicalY, physicalW, physicalH)
{
    initialize_();
}

/**
 * Destroys the render buffer.
 */
RenderBuffer::~RenderBuffer() = default;

/**
 * Removes all color render targets.
 */
void RenderBuffer::setRenderTargetColorNullAll()
{
    for (s32 i = 0; i < cRenderTargetColorMax; i++)
    {
        mRenderTargetColor[i] = nullptr;
    }
}

/**
 * Sets the virtual size and physical area to the size of a color render target.
 * @param colorIndex color render target index
 */
void RenderBuffer::adjustPhysicalAreaAndVirtualSizeFromColorTarget(u32 colorIndex)
{
    const RenderTargetColor* pTarget = mRenderTargetColor[s32(colorIndex)];
    u8 mipLevel = pTarget->getMipLevel();
    f32 width = u32(sead::Mathi::max(pTarget->getWidth() >> mipLevel, 1));
    f32 height = u32(sead::Mathi::max(pTarget->getMinHeight_(), pTarget->getHeight() >> mipLevel));
    setPhysicalArea(sead::BoundBox2f(sead::Vector2f(0.0f, 0.0f), sead::Vector2f(width, height)));
    setVirtualSize(sead::Vector2f(width, height));
}

/**
 * Invalidates the GPU texture cache for all render targets.
 * @param pDrawContext draw context
 */
void RenderBuffer::invalidateGPUCache(DrawContext* pDrawContext) const
{
    for (s32 i = 0; i < cRenderTargetColorMax; i++)
    {
        if (mRenderTargetColor[i] != nullptr)
        {
            mRenderTargetColor[i]->invalidateGPUCache(pDrawContext);
        }
    }

    if (mRenderTargetDepth != nullptr)
    {
        mRenderTargetDepth->invalidateGPUCache(pDrawContext);
    }
}

/**
 * Binds the render targets.
 * @param pDrawContext draw context
 * @param srgbBitmap color targets whose RGBA8 views are bound as sRGB
 */
void RenderBuffer::bind_(DrawContext* pDrawContext, u16 srgbBitmap) const
{
    pDrawContext->setBoundRenderBuffer(this);

    const NVNtexture* textures[cRenderTargetColorMax + 1] = {};
    const NVNtextureView* views[cRenderTargetColorMax + 1] = {};
    NVNtextureView srgbViews[cRenderTargetColorMax];

    s32 colorNum = 0;

    for (u32 i = 0; i < cRenderTargetColorMax; i++)
    {
        const RenderTargetColor* pTarget = getRenderTargetColor(i);

        if (pTarget == nullptr)
        {
            continue;
        }

        driver::NVNMgr::instance()->clearCompressedFrameBufferColor(pDrawContext, *pTarget);
        textures[i] = pTarget->getTexture().getTexture();
        pTarget->updateRegs_();
        views[i] = pTarget->getTextureView();

        if ((1 << i) & srgbBitmap)
        {
            srgbViews[i] = *pTarget->getTextureView();

            if (pTarget->getTextureFormat() ==
                u16(TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm))
            {
                nvnTextureViewSetFormat(&srgbViews[i], NVN_FORMAT_RGBA8_SRGB);
            }

            views[i] = &srgbViews[i];
        }

        colorNum = i + 1;
    }

    const RenderTargetDepth* pDepth = mRenderTargetDepth;

    if (pDepth != nullptr)
    {
        textures[cRenderTargetColorMax] = pDepth->getTexture().getTexture();
        pDepth->updateRegs_();
        views[cRenderTargetColorMax] = pDepth->getTextureView();
        driver::NVNMgr::instance()->clearCompressedFrameBufferDepth(pDrawContext,
                                                                    *mRenderTargetDepth);
    }

    nvnCommandBufferSetRenderTargets(pDrawContext->getNvnCommandBuffer(), colorNum, textures,
                                     views, textures[cRenderTargetColorMax],
                                     views[cRenderTargetColorMax]);
}

/**
 * Binds the render targets.
 * @param pDrawContext draw context
 */
void RenderBuffer::bindImpl_(sead::DrawContext* pDrawContext) const
{
    bind_(sead::DynamicCast<DrawContext>(pDrawContext), 0);
}

/**
 * Clears the first color target and the depth target.
 * @param pSeadDrawContext draw context
 * @param clearFlag clear flags
 * @param rColor clear color
 * @param depth clear depth
 * @param stencil clear stencil
 */
void RenderBuffer::clear(sead::DrawContext* pSeadDrawContext, u32 clearFlag,
                         const sead::Color4f& rColor, f32 depth, u32 stencil) const
{
    DrawContext* pDrawContext = sead::DynamicCast<DrawContext>(pSeadDrawContext);

    if (clearFlag == 0)
    {
        return;
    }

    clearTargets(this, pDrawContext, 0, clearFlag, rColor, depth, stencil);
}

/**
 * Clears a color target and the depth target.
 * @param pDrawContext draw context
 * @param target color target index
 * @param clearFlag clear flags
 * @param rColor clear color
 * @param depth clear depth
 * @param stencil clear stencil
 */
void RenderBuffer::clear(DrawContext* pDrawContext, u32 target, u32 clearFlag,
                         const sead::Color4f& rColor, f32 depth, u32 stencil) const
{
    if (clearFlag == 0)
    {
        return;
    }

    clearTargets(this, pDrawContext, target, clearFlag, rColor, depth, stencil);
}

/**
 * Updates all render targets, applies a viewport and clears a color target and the depth target.
 * @param pDrawContext draw context
 * @param target color target index
 * @param clearFlag clear flags
 * @param rColor clear color
 * @param depth clear depth
 * @param stencil clear stencil
 * @param rViewport viewport to apply
 * @param unused unused
 */
void RenderBuffer::fastClear(DrawContext* pDrawContext, u32 target, u32 clearFlag,
                             const sead::Color4f& rColor, f32 depth, u32 stencil,
                             const sead::Viewport& rViewport, bool unused) const
{
    if (mRenderTargetColor[s32(target)] == nullptr)
    {
        clearFlag &= ~cColor;
    }

    if (mRenderTargetDepth == nullptr)
    {
        clearFlag &= ~(cDepth | cStencil);
    }

    if (clearFlag == 0)
    {
        return;
    }

    for (u32 i = 0; i < cRenderTargetColorMax; i++)
    {
        const RenderTargetColor* pTarget = getRenderTargetColor(i);

        if (pTarget != nullptr)
        {
            pTarget->updateRegs_();
        }
    }

    if (mRenderTargetDepth != nullptr)
    {
        mRenderTargetDepth->updateRegs_();
    }

    rViewport.apply(pDrawContext, *this);
    clearTargets(this, pDrawContext, target, clearFlag, rColor, depth, stencil);
}

/**
 * Does nothing (no Y flip needed on NVN).
 * @param pDrawContext unused
 * @param flipX unused
 * @param flipY unused
 */
void RenderBuffer::drawFlipYGL_(DrawContext* pDrawContext, bool flipX, bool flipY) const {}

/**
 * Checks the render state.
 * @return true
 */
bool RenderBuffer::checkRenderState()
{
    return true;
}

/**
 * Initializes a texture with the format and size of a color target of the bound render buffer.
 * @param pDrawContext draw context
 * @param pTextureData texture to initialize
 * @param colorIndex color target index
 * @return whether the texture was initialized
 */
bool RenderBuffer::initTextureDataFromBoundColor(DrawContext* pDrawContext,
                                                 TextureData* pTextureData, s32 colorIndex)
{
    const RenderBuffer* pRenderBuffer = pDrawContext->getBoundRenderBuffer();

    if (pRenderBuffer == nullptr)
    {
        return false;
    }

    return pRenderBuffer->initTextureDataFromColor(pDrawContext, pTextureData, colorIndex);
}

/**
 * Initializes a texture with the format and size of a color target.
 * @param pDrawContext unused
 * @param pTextureData texture to initialize
 * @param colorIndex color target index
 * @return whether the texture was initialized
 */
bool RenderBuffer::initTextureDataFromColor(DrawContext* pDrawContext, TextureData* pTextureData,
                                            s32 colorIndex) const
{
    const RenderTargetColor* pTarget = mRenderTargetColor[colorIndex];

    if (pTarget == nullptr)
    {
        return false;
    }

    return initTextureDataFrom(pTarget, pTextureData);
}

/**
 * Initializes a texture with the format and size of the depth target of the bound render buffer.
 * @param pDrawContext draw context
 * @param pTextureData texture to initialize
 * @return whether the texture was initialized
 */
bool RenderBuffer::initTextureDataFromBoundDepth(DrawContext* pDrawContext,
                                                 TextureData* pTextureData)
{
    const RenderBuffer* pRenderBuffer = pDrawContext->getBoundRenderBuffer();

    if (pRenderBuffer == nullptr)
    {
        return false;
    }

    return pRenderBuffer->initTextureDataFromDepth(pDrawContext, pTextureData);
}

/**
 * Initializes a texture with the format and size of the depth target.
 * @param pDrawContext unused
 * @param pTextureData texture to initialize
 * @return whether the texture was initialized
 */
bool RenderBuffer::initTextureDataFromDepth(DrawContext* pDrawContext,
                                            TextureData* pTextureData) const
{
    const RenderTargetDepth* pTarget = mRenderTargetDepth;

    if (pTarget == nullptr)
    {
        return false;
    }

    return initTextureDataFrom(pTarget, pTextureData);
}

/**
 * Copies a color target of the bound render buffer into a texture.
 * @param pDrawContext draw context
 * @param pTextureData destination texture
 * @param colorIndex color target index
 * @return whether the copy was made
 */
bool RenderBuffer::copyTextureDataFromBoundColor(DrawContext* pDrawContext,
                                                 const TextureData* pTextureData,
                                                 s32 colorIndex)
{
    const RenderBuffer* pRenderBuffer = pDrawContext->getBoundRenderBuffer();

    if (pRenderBuffer == nullptr)
    {
        return false;
    }

    pRenderBuffer->mRenderTargetColor[colorIndex]->invalidateGPUCache(pDrawContext);
    return pRenderBuffer->copyTextureDataFromColor(pDrawContext, pTextureData, colorIndex);
}

/**
 * Copies a color target into a texture.
 * @param pDrawContext draw context
 * @param pTextureData destination texture
 * @param colorIndex color target index
 * @return whether the copy was made
 */
bool RenderBuffer::copyTextureDataFromColor(DrawContext* pDrawContext,
                                            const TextureData* pTextureData,
                                            s32 colorIndex) const
{
    const RenderTargetColor* pTarget = mRenderTargetColor[colorIndex];

    if (pTarget == nullptr)
    {
        return false;
    }

    pTarget->copyToAll(pDrawContext, pTextureData);
    return true;
}

/**
 * Copies the depth target of the bound render buffer into a texture.
 * @param pDrawContext draw context
 * @param pTextureData destination texture
 * @return whether the copy was made
 */
bool RenderBuffer::copyTextureDataFromBoundDepth(DrawContext* pDrawContext,
                                                 const TextureData* pTextureData)
{
    const RenderBuffer* pRenderBuffer = pDrawContext->getBoundRenderBuffer();

    if (pRenderBuffer == nullptr)
    {
        return false;
    }

    pRenderBuffer->mRenderTargetDepth->invalidateGPUCache(pDrawContext);
    return pRenderBuffer->copyTextureDataFromDepth(pDrawContext, pTextureData);
}

/**
 * Copies the depth target into a texture.
 * @param pDrawContext draw context
 * @param pTextureData destination texture
 * @return whether the copy was made
 */
bool RenderBuffer::copyTextureDataFromDepth(DrawContext* pDrawContext,
                                            const TextureData* pTextureData) const
{
    const RenderTargetDepth* pTarget = mRenderTargetDepth;

    if (pTarget == nullptr)
    {
        return false;
    }

    pTarget->expandHiZBufferTo(pDrawContext, pTextureData, pTarget->getSlice(),
                               pTarget->getMipLevel());
    return true;
}

/**
 * Checks whether a texture has a valid size for a color target.
 * @param pTextureData texture
 * @param colorIndex color target index
 * @return true
 */
bool RenderBuffer::checkValidTextureSize_(const TextureData& pTextureData, s32 colorIndex) const
{
    return true;
}

}  // namespace agl

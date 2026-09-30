#include "common/aglRenderTarget.h"

#include <math/seadMathCalcCommon.h>
#include <nvn/nvn_FuncPtrInline.h>

#include "driver/aglNVNMgr.h"

namespace agl {

/**
 * Constructs an empty color render target.
 */
RenderTargetColor::RenderTargetColor() = default;

/**
 * Constructs a color render target for a mip level and slice of a texture.
 * @param rTextureData texture to render into
 * @param mipLevel mip level to render into
 * @param slice slice to render into
 */
RenderTargetColor::RenderTargetColor(const TextureData& rTextureData, u32 mipLevel, u32 slice)
    : RenderTarget(mipLevel, slice)
{
    applyTextureData(rTextureData, mMipLevel, mSlice);
}

/**
 * Destroys the color render target.
 */
RenderTargetColor::~RenderTargetColor() = default;

/**
 * Constructs an empty depth render target.
 */
RenderTargetDepth::RenderTargetDepth() = default;

/**
 * Constructs a depth render target for a mip level and slice of a texture.
 * @param rTextureData texture to render into
 * @param mipLevel mip level to render into
 * @param slice slice to render into
 */
RenderTargetDepth::RenderTargetDepth(const TextureData& rTextureData, u32 mipLevel, u32 slice)
    : RenderTarget(mipLevel, slice)
{
    applyTextureData(rTextureData, mMipLevel, mSlice);
}

/**
 * Destroys the depth render target.
 */
RenderTargetDepth::~RenderTargetDepth() = default;

/**
 * Updates the render target state after the texture changed.
 */
void RenderTargetColor::onApplyTextureData_()
{
    mFlags.change(1, getTextureType() == u16(TextureType::cTextureType_2D_Array));
}

/**
 * Updates the Z-cull storage requirements after the texture changed.
 */
void RenderTargetDepth::onApplyTextureData_()
{
    s32 alignment;
    nvnDeviceGetInteger(driver::NVNMgr::instance()->getNvnDevice(),
                        NVN_DEVICE_INFO_ZCULL_SAVE_RESTORE_ALIGNMENT, &alignment);
    mZCullAlignment = alignment;

    if (getImagePtr().isValid())
    {
        mZCullSize = nvnTextureGetZCullStorageSize(getTexture().getTexture());
    }
    else
    {
        NVNtextureBuilder builder;
        initializeNVNtextureBuilder(&builder);
        mZCullSize = nvnTextureBuilderGetZCullStorageSize(&builder);
    }
}

/**
 * Sets up the texture view for the mip level and slice.
 * @param index unused
 */
void RenderTargetColor::initRegs_(u32 index) const
{
    NVNtextureView* pView = const_cast<NVNtextureView*>(&mTextureView);
    nvnTextureViewSetLevels(pView, mMipLevel, 1);
    nvnTextureViewSetLayers(pView, mSlice, 1);
}

/**
 * Sets up the texture view for the mip level and slice.
 * @param index unused
 */
void RenderTargetDepth::initRegs_(u32 index) const
{
    NVNtextureView* pView = const_cast<NVNtextureView*>(&mTextureView);
    nvnTextureViewSetLevels(pView, mMipLevel, 0);
    nvnTextureViewSetLayers(pView, mSlice, 1);
}

/**
 * Invalidates the GPU texture cache for the render target.
 * @param pDrawContext draw context
 */
void RenderTargetColor::invalidateGPUCache(DrawContext* pDrawContext) const
{
    driver::NVNMgr::instance()->invalidateGPUCacheColor(pDrawContext, getTextureID());
}

/**
 * Invalidates the CPU cache of the texture memory.
 */
void RenderTargetColor::invalidateCPUCache() const
{
    TextureData::invalidateCPUCache();
}

/**
 * Invalidates the GPU texture cache for the render target.
 * @param pDrawContext draw context
 */
void RenderTargetDepth::invalidateGPUCache(DrawContext* pDrawContext) const
{
    driver::NVNMgr::instance()->invalidateGPUCacheDepth(pDrawContext, getTextureID());
}

/**
 * Invalidates the CPU cache of the texture memory and the Z-cull buffer.
 */
void RenderTargetDepth::invalidateCPUCache() const
{
    TextureData::invalidateCPUCache();

    if (mZCullBuffer.isValid())
    {
        mZCullBuffer.invalidateCPUCache(mZCullSize);
    }
}

/**
 * Does nothing (no auxiliary buffer on NVN).
 * @param pDrawContext unused
 */
void RenderTargetColor::expandAuxBuffer(DrawContext* pDrawContext) const {}

/**
 * Does nothing (no hierarchical Z buffer on NVN).
 * @param pDrawContext unused
 */
void RenderTargetDepth::expandHiZBuffer(DrawContext* pDrawContext) const {}

/**
 * Copies the depth buffer into another texture if Z-cull memory exists.
 * @param pDrawContext draw context
 * @param pDst destination texture
 * @param dstSlice destination slice
 * @param dstMipLevel destination mip level
 */
void RenderTargetDepth::expandHiZBufferTo(DrawContext* pDrawContext, const TextureData* pDst,
                                          u32 dstSlice, u32 dstMipLevel) const
{
    if (mZCullBuffer.isValid())
    {
        copyTo(pDrawContext, pDst, dstSlice, dstMipLevel, mSlice, mMipLevel);
    }
}

/**
 * Does nothing (no hierarchical Z buffer on NVN).
 * @param pDrawContext unused
 */
void RenderTargetDepth::expandHiZBufferAllSlice(DrawContext* pDrawContext) const {}

/**
 * Copies every slice of the depth buffer into another texture.
 * @param pDrawContext draw context
 * @param pDst destination texture
 */
void RenderTargetDepth::expandHiZBufferToAllSlice(DrawContext* pDrawContext,
                                                  const TextureData* pDst) const
{
    s32 sliceNum = sead::Mathi::max(pDst->getMinSlice_(), pDst->getDepth());

    for (s32 i = 0; i < sliceNum; i++)
    {
        copyTo(pDrawContext, pDst, i, mMipLevel, i, mMipLevel);
    }
}

}  // namespace agl

#include "common/aglTextureData.h"
#include <nvn/nvn_FuncPtrInline.h>
#include "common/aglDrawContext.h"
#include "common/aglTextureFormatInfo.h"
#include "common/aglTextureSampler.h"
#include "detail/aglGPUMemBlockMgr.h"
#include "detail/aglTextureDataUtil.h"

namespace agl {

/**
 * Constructs an empty texture.
 */
TextureData::TextureData() : mTextureFormat(0), mMaxMipLevel(1), mDebugLabel("agl::TextureData") {}

/**
 * Sets up the texture description.
 * @param type texture type
 * @param format texture format
 * @param width width in pixels
 * @param height height in pixels
 * @param slice depth or slice count
 * @param mipLevelNum requested number of mip levels
 * @param attribute texture attribute flags
 * @param multiSampleType multisample type
 * @param calcSize whether to calculate the storage size and alignment
 */
void TextureData::initialize_(TextureType type, TextureFormat format, u32 width, u32 height,
                              u32 slice, u32 mipLevelNum, TextureAttribute attribute,
                              MultiSampleType multiSampleType, bool calcSize)
{
    mTextureFormat = static_cast<u16>(format);
    mSurface.initialize(type, format, mipLevelNum, attribute, multiSampleType);
    mImagePtr.invalidate();
    mMipPtr.invalidate();
    initializeSize_(width, height, slice);
    setMipLevelNum_(mipLevelNum, calcSize);
}

/**
 * Sets the dimensions of the texture and calculates the maximum number of mip levels.
 * @param width width in pixels
 * @param height height in pixels
 * @param slice depth or slice count
 */
void TextureData::initializeSize_(u32 width, u32 height, u32 slice)
{
    mSurface.initializeSize(width, height, slice);

    if (isMultiSample()) {
        mMaxMipLevel = 1;
        return;
    }

    s32 level = 1;
    while (getMipWidth(level) != getMipWidth(level - 1) ||
           getMipHeight(level) != getMipHeight(level - 1) ||
           getMipSlice(level) != getMipSlice(level - 1)) {
        level++;
    }

    mMaxMipLevel = level;
}

/**
 * Sets the number of mip levels, clamped to the valid range.
 * @param mipLevelNum requested number of mip levels
 * @param calcSize whether to recalculate the storage size and alignment
 */
void TextureData::setMipLevelNum_(u32 mipLevelNum, bool calcSize)
{
    if (isMultiSample()) {
        mSurface.mLevels = 1;
    } else {
        s32 level = mipLevelNum;
        s32 max = mMaxMipLevel;
        mSurface.mLevels = level < 1 ? 1 : (max < level ? max : level);
    }

    if (calcSize) {
        mSurface.calcSizeAndAlignment();
    }
}

/**
 * Gets the minimum height of a mip level (the layer count for 1D array textures).
 * @return the minimum height
 */
u32 TextureData::getMinHeight_() const
{
    if (mSurface.mTarget == NVN_TEXTURE_TARGET_1D_ARRAY) {
        return mSurface.mHeight;
    }

    return 1;
}

/**
 * Gets the minimum depth of a mip level (the layer count for array and cube map textures).
 * @return the minimum depth
 */
u32 TextureData::getMinSlice_() const
{
    switch (mSurface.mTarget) {
    case NVN_TEXTURE_TARGET_2D_ARRAY:
    case NVN_TEXTURE_TARGET_2D_MULTISAMPLE_ARRAY:
    case NVN_TEXTURE_TARGET_CUBEMAP:
        return mSurface.mDepth;
    default:
        return 1;
    }
}

/**
 * Gets the name of the texture format.
 * @return the format name
 */
sead::SafeString TextureData::getTextureFormatName() const
{
    return TextureFormatInfo::getString(TextureFormat(mTextureFormat));
}

/**
 * Gets the storage size of a mip level.
 * @param mipLevel mip level
 * @return the size of the whole texture for level 0, otherwise 0
 */
u32 TextureData::calcMipByteSize(u32 mipLevel) const
{
    if (mipLevel != 0) {
        return 0;
    }

    return mSurface.mStorageSize;
}

/**
 * Checks whether the texture format is block compressed.
 * @return true if compressed
 */
bool TextureData::isCompressedFormat() const
{
    return TextureFormatInfo::isCompressed(TextureFormat(mTextureFormat));
}

/**
 * Checks whether render target compression can be used with the texture format.
 * @return true if available
 */
bool TextureData::isRenderTargetCompressAvailable() const
{
    return TextureFormatInfo::isRenderTargetCompressAvailable(TextureFormat(mTextureFormat));
}

/**
 * Checks whether the texture format is a depth format.
 * @return true if depth
 */
bool TextureData::isDepthFormat() const
{
    switch (TextureFormat(mTextureFormat)) {
    case TextureFormat::cTextureFormat_Depth_16:
    case TextureFormat::cTextureFormat_Depth_32:
    case TextureFormat::cTextureFormat_Depth_24_uNorm_Stencil_8:
    case TextureFormat::cTextureFormat_Depth_32_float_Stencil_8:
        return true;
    default:
        return false;
    }
}

/**
 * Checks whether the texture format has a stencil component.
 * @return true if it has stencil
 */
bool TextureData::hasStencil() const
{
    switch (TextureFormat(mTextureFormat)) {
    case TextureFormat::cTextureFormat_Depth_24_uNorm_Stencil_8:
    case TextureFormat::cTextureFormat_Depth_32_float_Stencil_8:
        return true;
    default:
        return false;
    }
}

/**
 * Invalidates the CPU cache of the image and mip storage.
 */
void TextureData::invalidateCPUCache() const
{
    if (mImagePtr.isValid()) {
        GPUMemVoidAddr(mImagePtr).invalidateCPUCache(mSurface.mStorageSize);
    }

    if (mMipPtr.isValid()) {
        GPUMemVoidAddr(mMipPtr).invalidateCPUCache(mSurface._14);
    }
}

/**
 * Flushes the CPU cache of the image and mip storage.
 */
void TextureData::flushCPUCache() const
{
    if (mImagePtr.isValid()) {
        GPUMemVoidAddr(mImagePtr).flushCPUCache(mSurface.mStorageSize);
    }

    if (mMipPtr.isValid()) {
        GPUMemVoidAddr(mMipPtr).flushCPUCache(mSurface._14);
    }
}

/**
 * Copies every mip level and slice that both textures have to another texture.
 * @param pDrawContext draw context
 * @param pDst destination texture
 */
void TextureData::copyToAll(DrawContext* pDrawContext, const TextureData* pDst) const
{
    const u32 mip_num = mSurface.mLevels < pDst->mSurface.mLevels ? mSurface.mLevels :
                                                                     pDst->mSurface.mLevels;
    for (u32 mip = 0; mip < mip_num; mip++) {
        const u32 src_slice_num = getMipSlice(mip);
        const u32 dst_slice_num = pDst->getMipSlice(mip);
        const u32 slice_num = src_slice_num < dst_slice_num ? src_slice_num : dst_slice_num;
        for (u32 slice = 0; slice < slice_num; slice++) {
            copyTo_(pDrawContext, pDst, slice, mip, slice, mip, false);
        }
    }
}

/**
 * Copies one mip level slice to another texture.
 * @param pDrawContext draw context
 * @param pDst destination texture
 * @param dstSlice destination slice
 * @param dstMipLevel destination mip level
 * @param srcSlice source slice
 * @param srcMipLevel source mip level
 */
void TextureData::copyTo_(DrawContext* pDrawContext, const TextureData* pDst, s32 dstSlice,
                          s32 dstMipLevel, s32 srcSlice, s32 srcMipLevel, bool) const
{
    const s32 src_width = getMipWidth(srcMipLevel);
    const s32 src_height = getMipHeight(srcMipLevel);
    const s32 dst_width = pDst->getMipWidth(dstMipLevel);
    const s32 dst_height = pDst->getMipHeight(dstMipLevel);
    NVNcopyRegion src_region = {0, 0, srcSlice, src_width, src_height, 1};
    NVNcopyRegion dst_region = {0, 0, dstSlice, dst_width, dst_height, 1};

    NVNtextureView src_view;
    nvnTextureViewSetDefaults(&src_view);
    nvnTextureViewSetLevels(&src_view, srcMipLevel, 1);

    NVNtextureView dst_view;
    nvnTextureViewSetDefaults(&dst_view);
    nvnTextureViewSetLevels(&dst_view, dstMipLevel, 1);

    nvnCommandBufferCopyTextureToTexture(pDrawContext->getNvnCommandBuffer(),
                                         mTexture.getTexture(), &src_view, &src_region,
                                         pDst->mTexture.getTexture(), &dst_view, &dst_region, 0);
}

/**
 * Copies one mip level slice to the same mip level slice of another texture.
 * @param pDrawContext draw context
 * @param pDst destination texture
 * @param slice slice
 * @param mipLevel mip level
 */
void TextureData::copyTo(DrawContext* pDrawContext, const TextureData* pDst, s32 slice,
                         s32 mipLevel) const
{
    copyTo_(pDrawContext, pDst, slice, mipLevel, slice, mipLevel, false);
}

/**
 * Copies one mip level slice to another texture.
 * @param pDrawContext draw context
 * @param pDst destination texture
 * @param dstSlice destination slice
 * @param dstMipLevel destination mip level
 * @param srcSlice source slice
 * @param srcMipLevel source mip level
 */
void TextureData::copyTo(DrawContext* pDrawContext, const TextureData* pDst, s32 dstSlice,
                         s32 dstMipLevel, s32 srcSlice, s32 srcMipLevel) const
{
    copyTo_(pDrawContext, pDst, dstSlice, dstMipLevel, srcSlice, srcMipLevel, false);
}

/**
 * Sets the label used when registering the texture.
 * @param rDebugLabel label
 */
void TextureData::setDebugLabel(const sead::SafeString& rDebugLabel)
{
    mDebugLabel = rDebugLabel.cstr();
}

/**
 * Gets the label used when registering the texture.
 * @return the label
 */
sead::SafeString TextureData::getDebugLabel() const
{
    return mDebugLabel;
}

/**
 * Sets the image storage and recreates the NVN texture.
 * @param imagePtr image storage
 * @param releaseOnly if set, only releases the NVN texture
 */
void TextureData::setImagePtr(GPUMemVoidAddr imagePtr, u32 releaseOnly)
{
    const GPUMemVoidAddr old = mImagePtr;
    mImagePtr = imagePtr;

    if (!(releaseOnly & 1)) {
        if (old.getMemoryPool() == mImagePtr.getMemoryPool() &&
            mImagePtr.getByteOffset() == old.getByteOffset() &&
            old.getMemoryBlock() == mImagePtr.getMemoryBlock()) {
            return;
        }

        if (mImagePtr.isValid()) {
            updateNVNtexture();
            return;
        }
    }

    mTexture.releaseTexture();
}

/**
 * Recreates and registers the NVN texture from the texture description and image storage.
 */
void TextureData::updateNVNtexture()
{
    if (!mImagePtr.isValid()) {
        return;
    }

    NVNmemoryPool* pool = mImagePtr.getMemoryPool()->getDriverPool();
    const u32 offset = mImagePtr.getByteOffset();

    NVNtextureBuilder builder;
    NVNtexture texture = {};
    mSurface.setupNVNtextureBuilder(&builder);
    nvnTextureBuilderSetStorage(&builder, pool, offset);
    nvnTextureInitialize(&texture, &builder);

    if (!mTexture.registerTexture(&texture, nullptr, mDebugLabel, true)) {
        nvnTextureFinalize(&texture);
    }
}

/**
 * Uses the image storage of another texture.
 * @param rOther texture to share the image storage of
 */
void TextureData::shareImagePtr(const TextureData& rOther)
{
    setImagePtr(rOther.mImagePtr);
}

/**
 * Sets the mip storage.
 * @param mipPtr mip storage
 */
void TextureData::setMipPtr(GPUMemVoidAddr mipPtr)
{
    mMipPtr = mipPtr;
}

/**
 * Sets the component selectors and recreates the NVN texture.
 * @param r red selector
 * @param g green selector
 * @param b blue selector
 * @param a alpha selector
 */
// NON_MATCHING: scheduling of the image pointer offset load
void TextureData::setCompSel(TextureCompSel r, TextureCompSel g, TextureCompSel b,
                             TextureCompSel a)
{
    union {
        detail::CompSelData data;
        u32 raw;
    } comp_sel;
    comp_sel.data.mR = r;
    comp_sel.data.mG = g;
    comp_sel.data.mB = b;
    comp_sel.data.mA = a;
    *reinterpret_cast<u32*>(&mSurface.mCompSel) = comp_sel.raw;
    updateNVNtexture();
}

/**
 * Resets the component selectors to the format defaults and recreates the NVN texture.
 */
void TextureData::setCompSelDefault()
{
    {
        detail::CompSelData comp_sel;
        reinterpret_cast<detail::CompSel&>(comp_sel).setDefault(TextureFormat(mTextureFormat));
        mSurface.mCompSel = comp_sel;
    }

    updateNVNtexture();
}

/**
 * Sets up the texture from an initialized NVN texture and registers it.
 * @param rTexture NVN texture
 */
void TextureData::initializeFromNVNtexture(const NVNtexture& rTexture)
{
    mSurface.copyFrom(rTexture);
    mTextureFormat = static_cast<u16>(
        detail::TextureDataUtil::convFormatDriverToAGL(NVNformat(mSurface.mFormat)));
    initializeSize_(mSurface.mWidth, mSurface.mHeight, mSurface.mDepth);
    setMipLevelNum_(mSurface.mLevels, true);
    mTexture.registerTexture(&rTexture, nullptr, mDebugLabel, false);
}

/**
 * Initializes an NVN texture from the texture description.
 * @param pTexture NVN texture to initialize
 * @param imagePtr image storage
 */
void TextureData::initializeNVNtexture(NVNtexture* pTexture, GPUMemVoidAddr imagePtr) const
{
    *pTexture = {};
    NVNtextureBuilder builder;
    mSurface.setupNVNtextureBuilder(&builder);
    nvnTextureBuilderSetStorage(&builder, imagePtr.getMemoryPool()->getDriverPool(),
                                imagePtr.getByteOffset());
    nvnTextureInitialize(pTexture, &builder);
}

/**
 * Registers the NVN texture with a texture view.
 * @param pView texture view
 */
void TextureData::updateNVNtextureView(const NVNtextureView* pView)
{
    mTexture.registerTexture(nullptr, pView, mDebugLabel, true);
}

/**
 * Sets up an NVN texture builder from the texture description.
 * @param pBuilder texture builder
 */
void TextureData::initializeNVNtextureBuilder(NVNtextureBuilder* pBuilder) const
{
    mSurface.setupNVNtextureBuilder(pBuilder);
}

/**
 * Initializes an nn::gfx texture from the NVN texture.
 * @param pTexture nn::gfx texture
 */
void TextureData::initializeGfxTexture(nn::gfx::Texture* pTexture) const
{
    mTexture.initializeGfxTexture(pTexture);
}

/**
 * Releases the NVN texture.
 */
void TextureData::releaseNVNtexture()
{
    mTexture.~NVNtexture_();
    new (&mTexture) driver::NVNtexture_();
}

/**
 * Switches a UNorm format to its sRGB variant (not supported on this platform).
 */
void TextureData::forceChangeUNormToSRGB() {}

/**
 * Switches an sRGB format to its UNorm variant (not supported on this platform).
 */
void TextureData::forceChangeSRGBToUNorm() {}

/**
 * Checks whether the texture format is an sRGB format.
 * @return true if sRGB
 */
bool TextureData::isSRGB() const
{
    return TextureFormatInfo::isSRGB(TextureFormat(mTextureFormat));
}

/**
 * Checks whether the texture format stores unsigned normalized values.
 * @return true if UNorm
 */
bool TextureData::isUNorm() const
{
    return TextureFormatInfo::isNormalized(TextureFormat(mTextureFormat)) &&
           TextureFormatInfo::isUnsigned(TextureFormat(mTextureFormat));
}

/**
 * Checks whether the texture format has an sRGB variant.
 * @return true if it has one
 */
bool TextureData::isEnableChangeToSRGB() const
{
    switch (TextureFormat(mTextureFormat)) {
    case TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm:
    case TextureFormat::cTextureFormat_BC1_uNorm:
    case TextureFormat::cTextureFormat_BC2_uNorm:
    case TextureFormat::cTextureFormat_BC3_uNorm:
        return true;
    default:
        return false;
    }
}

/**
 * Checks whether the texture format has a UNorm variant.
 * @return true if it has one
 */
bool TextureData::isEnableChangeToUNorm() const
{
    switch (TextureFormat(mTextureFormat)) {
    case TextureFormat::cTextureFormat_R8_G8_B8_A8_SRGB:
    case TextureFormat::cTextureFormat_BC1_SRGB:
    case TextureFormat::cTextureFormat_BC2_SRGB:
    case TextureFormat::cTextureFormat_BC3_SRGB:
        return true;
    default:
        return false;
    }
}

/**
 * Sets up the texture as a cube map array.
 * @param format texture format
 * @param width width in pixels
 * @param height height in pixels
 * @param arrayNum number of cube maps
 * @param mipLevelNum requested number of mip levels
 * @param attribute texture attribute flags
 */
void TextureData::initializeCubeMapArray(TextureFormat format, u32 width, u32 height,
                                         u32 arrayNum, u32 mipLevelNum,
                                         TextureAttribute attribute)
{
    const u32 slice = arrayNum * 6;
    mTextureFormat = static_cast<u16>(format);
    mSurface.initialize(TextureType(NVN_TEXTURE_TARGET_CUBEMAP), format, mipLevelNum, attribute,
                        MultiSampleType(0));
    mImagePtr.invalidate();
    mMipPtr.invalidate();
    initializeSize_(width, height, slice);
    setMipLevelNum_(mipLevelNum, true);
}

}  // namespace agl

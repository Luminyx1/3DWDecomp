#include <cstring>
#include <nvn/nvn_FuncPtrInline.h>
#include "common/aglTextureFormatInfo.h"
#include "common/aglTextureSampler.h"
#include "detail/aglSurface.h"
#include "detail/aglTextureDataUtil.h"
#include "driver/aglNVNMgr.h"

namespace agl::detail {

/**
 * Constructs a component selector that passes red, green, blue and alpha through.
 */
CompSel::CompSel()
{
    mR = cTextureCompSel_R;
    mG = cTextureCompSel_G;
    mB = cTextureCompSel_B;
    mA = cTextureCompSel_A;
}

/**
 * Sets the component selectors to the defaults of a texture format.
 * @param format texture format
 */
void CompSel::setDefault(TextureFormat format)
{
    mR = TextureFormatInfo::getDefaultCompSel(format, 0);
    mG = TextureFormatInfo::getDefaultCompSel(format, 1);
    mB = TextureFormatInfo::getDefaultCompSel(format, 2);
    mA = TextureFormatInfo::getDefaultCompSel(format, 3);
}

/**
 * Constructs an empty surface.
 */
Surface::Surface()
{
    std::memset(this, 0, sizeof(Surface));
}

/**
 * Sets up the format, type and mip level count of the surface.
 * @param type texture type
 * @param format texture format
 * @param mipLevelNum number of mip levels
 * @param attribute texture attribute flags
 * @param multiSampleType multisample type
 */
void Surface::initialize(TextureType type, TextureFormat format, u32 mipLevelNum,
                         TextureAttribute attribute, MultiSampleType multiSampleType)
{
    getCompSel().setDefault(format);
    mAttribute = static_cast<u8>(attribute);
    mPixelByteSize = TextureFormatInfo::getPixelByteSize(format);
    mTarget = static_cast<u16>(type);
    mFormat = TextureDataUtil::convFormatAGLToDriver(format);
    mSamples = static_cast<u8>(multiSampleType);
    mLevels = mipLevelNum;
}

/**
 * Sets the dimensions of the surface.
 * @param width width in pixels
 * @param height height in pixels
 * @param depth depth or slice count
 */
void Surface::initializeSize(u32 width, u32 height, u32 depth)
{
    mWidth = width;
    mHeight = height;
    mDepth = depth;
}

/**
 * Copies the base surface description.
 * @param rBase surface description to copy
 */
void Surface::copyFrom(const SurfaceBase& rBase)
{
    *static_cast<SurfaceBase*>(this) = rBase;
}

/**
 * Calculates the stride, storage class, size and alignment of the surface.
 */
void Surface::calcSizeAndAlignment()
{
    NVNtextureBuilder builder;
    {
        int alignment;
        nvnDeviceGetInteger(driver::NVNMgr::instance()->getNvnDevice(),
                            NVN_DEVICE_INFO_LINEAR_RENDER_TARGET_STRIDE_ALIGNMENT, &alignment);
        mStride = (mWidth * mPixelByteSize + alignment - 1) & -alignment;
    }

    setupNVNtextureBuilder(&builder);
    mStorageClass = nvnTextureBuilderGetStorageClass(&builder);
    mStorageSize = nvnTextureBuilderGetStorageSize(&builder);
    mAlignment = nvnTextureBuilderGetStorageAlignment(&builder);

    if (driver::NVNMgr::instance()->isPrintTextureInfo()) {
        nvnTextureBuilderGetFlags(&builder);
    }
}

/**
 * Sets up an NVN texture builder from the surface description.
 * @param pBuilder texture builder to set up
 */
void Surface::setupNVNtextureBuilder(NVNtextureBuilder* pBuilder) const
{
    nvnTextureBuilderSetDevice(pBuilder, driver::NVNMgr::instance()->getNvnDevice());
    nvnTextureBuilderSetDefaults(pBuilder);

    NVNtextureTarget target = NVNtextureTarget(mTarget);
    if (target == NVN_TEXTURE_TARGET_CUBEMAP) {
        target = mDepth > 6 ? NVN_TEXTURE_TARGET_CUBEMAP_ARRAY : NVN_TEXTURE_TARGET_CUBEMAP;
    }

    const NVNformat format = NVNformat(mFormat);
    nvnTextureBuilderSetTarget(pBuilder, target);
    nvnTextureBuilderSetWidth(pBuilder, mWidth);
    nvnTextureBuilderSetHeight(pBuilder, mHeight);
    nvnTextureBuilderSetDepth(pBuilder, mDepth);
    nvnTextureBuilderSetLevels(pBuilder, mLevels);
    nvnTextureBuilderSetFormat(pBuilder, format);
    const CompSel& comp_sel = getCompSel();
    nvnTextureBuilderSetSwizzle(pBuilder, NVNtextureSwizzle(comp_sel.mR),
                                NVNtextureSwizzle(comp_sel.mG), NVNtextureSwizzle(comp_sel.mB),
                                NVNtextureSwizzle(comp_sel.mA));
    nvnTextureBuilderSetSamples(pBuilder, mSamples);
    nvnTextureBuilderSetDepthStencilMode(pBuilder, NVN_TEXTURE_DEPTH_STENCIL_MODE_DEPTH);

    u32 flags = 0;
    if (mAttribute & cAttribute_Linear) {
        nvnTextureBuilderSetStride(pBuilder, mStride);
        flags = NVN_TEXTURE_FLAGS_LINEAR_RENDER_TARGET;
    }

    flags |= driver::NVNMgr::instance()->getTextureFlags((mAttribute & cAttribute_Compressible) != 0,
                                                          (mAttribute & cAttribute_RenderTarget) != 0,
                                                          format);
    nvnTextureBuilderSetFlags(pBuilder, flags);
}

/**
 * Prints the surface description (no-op in release builds).
 */
void Surface::printInfo() const {}

/**
 * Sets up the surface description from an initialized NVN texture.
 * @param rTexture NVN texture to copy from
 */
void Surface::copyFrom(const NVNtexture& rTexture)
{
    const int flags = nvnTextureGetFlags(&rTexture);
    mFormat = nvnTextureGetFormat(&rTexture);

    const u16 target = nvnTextureGetTarget(&rTexture);
    mTarget = target == NVN_TEXTURE_TARGET_CUBEMAP_ARRAY ? u16(NVN_TEXTURE_TARGET_CUBEMAP) : target;
    mSamples = nvnTextureGetSamples(&rTexture);

    mAttribute = ((flags >> 2) & cAttribute_Compressible) | ((flags >> 4) & cAttribute_Linear);
    mPixelByteSize = TextureFormatInfo::getPixelByteSize(
        TextureDataUtil::convFormatDriverToAGL(NVNformat(mFormat)));

    mWidth = nvnTextureGetWidth(&rTexture);
    mHeight = nvnTextureGetHeight(&rTexture);
    mDepth = nvnTextureGetDepth(&rTexture);
    mLevels = nvnTextureGetLevels(&rTexture);

    NVNtextureSwizzle r, g, b, a;
    nvnTextureGetSwizzle(&rTexture, &r, &g, &b, &a);
    CompSel comp_sel;
    comp_sel.mR = r;
    comp_sel.mG = g;
    comp_sel.mB = b;
    comp_sel.mA = a;
    getCompSel() = comp_sel;

    mStorageClass = nvnTextureGetStorageClass(&rTexture);
}

}  // namespace agl::detail

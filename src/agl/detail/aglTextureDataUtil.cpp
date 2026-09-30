#include "detail/aglTextureDataUtil.h"

namespace agl::detail {

namespace {

// clang-format off
const NVNformat cFormatTable[] = {
    NVN_FORMAT_NONE,  // Invalid
    NVN_FORMAT_R8,  // R8_uNorm
    NVN_FORMAT_R8UI,  // R8_uInt
    NVN_FORMAT_R8SN,  // R8_sNorm
    NVN_FORMAT_R8I,  // R8_sInt
    NVN_FORMAT_R16,  // R16_uNorm
    NVN_FORMAT_R16UI,  // R16_uInt
    NVN_FORMAT_R16SN,  // R16_sNorm
    NVN_FORMAT_R16I,  // R16_sInt
    NVN_FORMAT_R16F,  // R16_float
    NVN_FORMAT_RG8,  // R8_G8_uNorm
    NVN_FORMAT_RG8UI,  // R8_G8_uInt
    NVN_FORMAT_RG8SN,  // R8_G8_sNorm
    NVN_FORMAT_RG8I,  // R8_G8_sInt
    NVN_FORMAT_RGB565,  // R5_G6_B5_uNorm
    NVN_FORMAT_A1BGR5,  // A1_B5_G5_R5_uNorm
    NVN_FORMAT_RGBA4,  // R4_G4_B4_A4_uNorm
    NVN_FORMAT_RGB5A1,  // R5_G5_B5_A1_uNorm
    NVN_FORMAT_R32UI,  // R32_uInt
    NVN_FORMAT_R32I,  // R32_sInt
    NVN_FORMAT_R32F,  // R32_float
    NVN_FORMAT_RG16,  // R16_G16_uNorm
    NVN_FORMAT_RG16UI,  // R16_G16_uInt
    NVN_FORMAT_RG16SN,  // R16_G16_sNorm
    NVN_FORMAT_RG16I,  // R16_G16_sInt
    NVN_FORMAT_RG16F,  // R16_G16_float
    NVN_FORMAT_R11G11B10F,  // R11_G11_B10_float
    NVN_FORMAT_RGB10A2,  // A2_B10_G10_R10_uNorm
    NVN_FORMAT_RGB10A2UI,  // A2_B10_G10_R10_uInt
    NVN_FORMAT_RGBA8,  // R8_G8_B8_A8_uNorm
    NVN_FORMAT_RGBA8UI,  // R8_G8_B8_A8_uInt
    NVN_FORMAT_RGBA8SN,  // R8_G8_B8_A8_sNorm
    NVN_FORMAT_RGBA8I,  // R8_G8_B8_A8_sInt
    NVN_FORMAT_RGBA8_SRGB,  // R8_G8_B8_A8_SRGB
    NVN_FORMAT_RGB10A2,  // R10_G10_B10_A2_uNorm
    NVN_FORMAT_RGB10A2UI,  // R10_G10_B10_A2_uInt
    NVN_FORMAT_RG32UI,  // R32_G32_uInt
    NVN_FORMAT_RG32I,  // R32_G32_sInt
    NVN_FORMAT_RG32F,  // R32_G32_float
    NVN_FORMAT_RGBA16,  // R16_G16_B16_A16_uNorm
    NVN_FORMAT_RGBA16UI,  // R16_G16_B16_A16_uInt
    NVN_FORMAT_RGBA16SN,  // R16_G16_B16_A16_sNorm
    NVN_FORMAT_RGBA16I,  // R16_G16_B16_A16_sInt
    NVN_FORMAT_RGBA16F,  // R16_G16_B16_A16_float
    NVN_FORMAT_RGBA32UI,  // R32_G32_B32_A32_uInt
    NVN_FORMAT_RGBA32I,  // R32_G32_B32_A32_sInt
    NVN_FORMAT_RGBA32F,  // R32_G32_B32_A32_float
    NVN_FORMAT_RGBA_DXT1,  // BC1_uNorm
    NVN_FORMAT_RGBA_DXT1_SRGB,  // BC1_SRGB
    NVN_FORMAT_RGBA_DXT3,  // BC2_uNorm
    NVN_FORMAT_RGBA_DXT3_SRGB,  // BC2_SRGB
    NVN_FORMAT_RGBA_DXT5,  // BC3_uNorm
    NVN_FORMAT_RGBA_DXT5_SRGB,  // BC3_SRGB
    NVN_FORMAT_RGTC1_UNORM,  // BC4_uNorm
    NVN_FORMAT_RGTC1_SNORM,  // BC4_sNorm
    NVN_FORMAT_RGTC2_UNORM,  // BC5_uNorm
    NVN_FORMAT_RGTC2_SNORM,  // BC5_sNorm
    NVN_FORMAT_BPTC_UNORM,  // BC7_uNorm
    NVN_FORMAT_BPTC_UNORM_SRGB,  // BC7_SRGB
    NVN_FORMAT_DEPTH16,  // Depth_16
    NVN_FORMAT_DEPTH32F,  // Depth_32
    NVN_FORMAT_DEPTH24_STENCIL8,  // Depth_24_uNorm_Stencil_8
    NVN_FORMAT_DEPTH32F_STENCIL8,  // Depth_32_float_Stencil_8
};

// clang-format on

}  // namespace

/**
 * Converts an NVN texture format to the matching agl texture format.
 * @param format NVN texture format
 * @return the agl texture format, or cTextureFormat_Invalid if there is none
 */
TextureFormat TextureDataUtil::convFormatDriverToAGL(NVNformat format)
{
    for (s32 i = 0; i < static_cast<s32>(TextureFormat::cTextureFormat_Num); i++) {
        if (cFormatTable[i] == format) {
            return TextureFormat(i);
        }
    }

    return TextureFormat::cTextureFormat_Invalid;
}

/**
 * Converts an agl texture format to the matching NVN texture format.
 * @param format agl texture format
 * @return the NVN texture format
 */
NVNformat TextureDataUtil::convFormatAGLToDriver(TextureFormat format)
{
    return cFormatTable[static_cast<u32>(format)];
}

/**
 * Converts an NVN texture swizzle to the matching agl component selector.
 * @param swizzle NVN texture swizzle
 * @return the component selector
 */
TextureCompSel TextureDataUtil::convCompSelDriverToAGL(NVNtextureSwizzle swizzle)
{
    if (static_cast<u32>(swizzle) <= NVN_TEXTURE_SWIZZLE_A) {
        return TextureCompSel(swizzle);
    }

    return cTextureCompSel_0;
}

/**
 * Converts an nn::gfx image format to the matching agl texture format.
 * @param format nn::gfx image format
 * @return the agl texture format, or cTextureFormat_Invalid if there is none
 */
TextureFormat TextureDataUtil::convFormatNNGfxToAGL(nn::gfx::ImageFormat format)
{
    switch (format) {
    case nn::gfx::ImageFormat_R8_Unorm:
        return TextureFormat::cTextureFormat_R8_uNorm;
    case nn::gfx::ImageFormat_R8_Snorm:
        return TextureFormat::cTextureFormat_R8_sNorm;
    case nn::gfx::ImageFormat_R8_Uint:
        return TextureFormat::cTextureFormat_R8_uInt;
    case nn::gfx::ImageFormat_R8_Sint:
        return TextureFormat::cTextureFormat_R8_sInt;
    case nn::gfx::ImageFormat_R4_G4_B4_A4_Unorm:
        return TextureFormat::cTextureFormat_R4_G4_B4_A4_uNorm;
    case nn::gfx::ImageFormat_R5_G5_B5_A1_Unorm:
        return TextureFormat::cTextureFormat_R5_G5_B5_A1_uNorm;
    case nn::gfx::ImageFormat_A1_B5_G5_R5_Unorm:
        return TextureFormat::cTextureFormat_A1_B5_G5_R5_uNorm;
    case nn::gfx::ImageFormat_R5_G6_B5_Unorm:
        return TextureFormat::cTextureFormat_R5_G6_B5_uNorm;
    case nn::gfx::ImageFormat_R8_G8_Unorm:
        return TextureFormat::cTextureFormat_R8_G8_uNorm;
    case nn::gfx::ImageFormat_R8_G8_Snorm:
        return TextureFormat::cTextureFormat_R8_G8_sNorm;
    case nn::gfx::ImageFormat_R8_G8_Uint:
        return TextureFormat::cTextureFormat_R8_G8_uInt;
    case nn::gfx::ImageFormat_R8_G8_Sint:
        return TextureFormat::cTextureFormat_R8_G8_sInt;
    case nn::gfx::ImageFormat_R16_Unorm:
        return TextureFormat::cTextureFormat_R16_uNorm;
    case nn::gfx::ImageFormat_R16_Snorm:
        return TextureFormat::cTextureFormat_R16_sNorm;
    case nn::gfx::ImageFormat_R16_Uint:
        return TextureFormat::cTextureFormat_R16_uInt;
    case nn::gfx::ImageFormat_R16_Sint:
        return TextureFormat::cTextureFormat_R16_sInt;
    case nn::gfx::ImageFormat_R16_Float:
        return TextureFormat::cTextureFormat_R16_float;
    case nn::gfx::ImageFormat_D16_Unorm:
        return TextureFormat::cTextureFormat_Depth_16;
    case nn::gfx::ImageFormat_R8_G8_B8_A8_Unorm:
        return TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm;
    case nn::gfx::ImageFormat_R8_G8_B8_A8_Snorm:
        return TextureFormat::cTextureFormat_R8_G8_B8_A8_sNorm;
    case nn::gfx::ImageFormat_R8_G8_B8_A8_Uint:
        return TextureFormat::cTextureFormat_R8_G8_B8_A8_uInt;
    case nn::gfx::ImageFormat_R8_G8_B8_A8_Sint:
        return TextureFormat::cTextureFormat_R8_G8_B8_A8_sInt;
    case nn::gfx::ImageFormat_R8_G8_B8_A8_UnormSrgb:
        return TextureFormat::cTextureFormat_R8_G8_B8_A8_SRGB;
    case nn::gfx::ImageFormat_R10_G10_B10_A2_Unorm:
        return TextureFormat::cTextureFormat_R10_G10_B10_A2_uNorm;
    case nn::gfx::ImageFormat_R10_G10_B10_A2_Uint:
        return TextureFormat::cTextureFormat_R10_G10_B10_A2_uInt;
    case nn::gfx::ImageFormat_R11_G11_B10_Float:
        return TextureFormat::cTextureFormat_R11_G11_B10_float;
    case nn::gfx::ImageFormat_R16_G16_Unorm:
        return TextureFormat::cTextureFormat_R16_G16_uNorm;
    case nn::gfx::ImageFormat_R16_G16_Snorm:
        return TextureFormat::cTextureFormat_R16_G16_sNorm;
    case nn::gfx::ImageFormat_R16_G16_Uint:
        return TextureFormat::cTextureFormat_R16_G16_uInt;
    case nn::gfx::ImageFormat_R16_G16_Sint:
        return TextureFormat::cTextureFormat_R16_G16_sInt;
    case nn::gfx::ImageFormat_R16_G16_Float:
        return TextureFormat::cTextureFormat_R16_G16_float;
    case nn::gfx::ImageFormat_D24_Unorm_S8_Uint:
        return TextureFormat::cTextureFormat_Depth_24_uNorm_Stencil_8;
    case nn::gfx::ImageFormat_R32_Uint:
        return TextureFormat::cTextureFormat_R32_uInt;
    case nn::gfx::ImageFormat_R32_Sint:
        return TextureFormat::cTextureFormat_R32_sInt;
    case nn::gfx::ImageFormat_R32_Float:
        return TextureFormat::cTextureFormat_R32_float;
    case nn::gfx::ImageFormat_D32_Float:
        return TextureFormat::cTextureFormat_Depth_32;
    case nn::gfx::ImageFormat_R16_G16_B16_A16_Unorm:
        return TextureFormat::cTextureFormat_R16_G16_B16_A16_uNorm;
    case nn::gfx::ImageFormat_R16_G16_B16_A16_Snorm:
        return TextureFormat::cTextureFormat_R16_G16_B16_A16_sNorm;
    case nn::gfx::ImageFormat_R16_G16_B16_A16_Uint:
        return TextureFormat::cTextureFormat_R16_G16_B16_A16_uInt;
    case nn::gfx::ImageFormat_R16_G16_B16_A16_Sint:
        return TextureFormat::cTextureFormat_R16_G16_B16_A16_sInt;
    case nn::gfx::ImageFormat_R16_G16_B16_A16_Float:
        return TextureFormat::cTextureFormat_R16_G16_B16_A16_float;
    case nn::gfx::ImageFormat_D32_Float_S8_Uint_X24:
        return TextureFormat::cTextureFormat_Depth_32_float_Stencil_8;
    case nn::gfx::ImageFormat_R32_G32_Uint:
        return TextureFormat::cTextureFormat_R32_G32_uInt;
    case nn::gfx::ImageFormat_R32_G32_Sint:
        return TextureFormat::cTextureFormat_R32_G32_sInt;
    case nn::gfx::ImageFormat_R32_G32_Float:
        return TextureFormat::cTextureFormat_R32_G32_float;
    case nn::gfx::ImageFormat_R32_G32_B32_A32_Uint:
        return TextureFormat::cTextureFormat_R32_G32_B32_A32_uInt;
    case nn::gfx::ImageFormat_R32_G32_B32_A32_Sint:
        return TextureFormat::cTextureFormat_R32_G32_B32_A32_sInt;
    case nn::gfx::ImageFormat_R32_G32_B32_A32_Float:
        return TextureFormat::cTextureFormat_R32_G32_B32_A32_float;
    case nn::gfx::ImageFormat_Bc1_Unorm:
        return TextureFormat::cTextureFormat_BC1_uNorm;
    case nn::gfx::ImageFormat_Bc1_UnormSrgb:
        return TextureFormat::cTextureFormat_BC1_SRGB;
    case nn::gfx::ImageFormat_Bc2_Unorm:
        return TextureFormat::cTextureFormat_BC2_uNorm;
    case nn::gfx::ImageFormat_Bc2_UnormSrgb:
        return TextureFormat::cTextureFormat_BC2_SRGB;
    case nn::gfx::ImageFormat_Bc3_Unorm:
        return TextureFormat::cTextureFormat_BC3_uNorm;
    case nn::gfx::ImageFormat_Bc3_UnormSrgb:
        return TextureFormat::cTextureFormat_BC3_SRGB;
    case nn::gfx::ImageFormat_Bc4_Unorm:
        return TextureFormat::cTextureFormat_BC4_uNorm;
    case nn::gfx::ImageFormat_Bc4_Snorm:
        return TextureFormat::cTextureFormat_BC4_sNorm;
    case nn::gfx::ImageFormat_Bc5_Unorm:
        return TextureFormat::cTextureFormat_BC5_uNorm;
    case nn::gfx::ImageFormat_Bc5_Snorm:
        return TextureFormat::cTextureFormat_BC5_sNorm;
    case nn::gfx::ImageFormat_Bc7_Unorm:
        return TextureFormat::cTextureFormat_BC7_uNorm;
    case nn::gfx::ImageFormat_Bc7_UnormSrgb:
        return TextureFormat::cTextureFormat_BC7_SRGB;
    default:
        return TextureFormat::cTextureFormat_Invalid;
    }
}

}  // namespace agl::detail

#include "common/aglTextureFormatInfo.h"
#include "detail/aglTextureDataUtil.h"

namespace agl {

namespace {

struct FormatInfo {
    u8 mComponentBitSize[4];
    s8 mComponentOrder[4];
    u8 mPixelByteSize;
    u8 mComponentNum;
    bool mIsCompressed;
    bool mIsNormalized;
    bool mIsFloat;
    bool mIsUnsigned;
    bool mIsUsableAsRenderTargetColor;
    bool mIsUsableAsRenderTargetDepth;
    bool mIsSRGB;
};

// clang-format off
const TextureCompSel cDefaultCompSel[][4] = {
    {cTextureCompSel_0, cTextureCompSel_0, cTextureCompSel_0, cTextureCompSel_0},  // Invalid
    {cTextureCompSel_R, cTextureCompSel_0, cTextureCompSel_0, cTextureCompSel_1},  // R8_uNorm
    {cTextureCompSel_R, cTextureCompSel_0, cTextureCompSel_0, cTextureCompSel_1},  // R8_uInt
    {cTextureCompSel_R, cTextureCompSel_0, cTextureCompSel_0, cTextureCompSel_1},  // R8_sNorm
    {cTextureCompSel_R, cTextureCompSel_0, cTextureCompSel_0, cTextureCompSel_1},  // R8_sInt
    {cTextureCompSel_R, cTextureCompSel_0, cTextureCompSel_0, cTextureCompSel_1},  // R16_uNorm
    {cTextureCompSel_R, cTextureCompSel_0, cTextureCompSel_0, cTextureCompSel_1},  // R16_uInt
    {cTextureCompSel_R, cTextureCompSel_0, cTextureCompSel_0, cTextureCompSel_1},  // R16_sNorm
    {cTextureCompSel_R, cTextureCompSel_0, cTextureCompSel_0, cTextureCompSel_1},  // R16_sInt
    {cTextureCompSel_R, cTextureCompSel_0, cTextureCompSel_0, cTextureCompSel_1},  // R16_float
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_0, cTextureCompSel_1},  // R8_G8_uNorm
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_0, cTextureCompSel_1},  // R8_G8_uInt
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_0, cTextureCompSel_1},  // R8_G8_sNorm
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_0, cTextureCompSel_1},  // R8_G8_sInt
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B, cTextureCompSel_1},  // R5_G6_B5_uNorm
    {cTextureCompSel_A, cTextureCompSel_B, cTextureCompSel_G, cTextureCompSel_R},  // A1_B5_G5_R5_uNorm
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B, cTextureCompSel_A},  // R4_G4_B4_A4_uNorm
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B, cTextureCompSel_A},  // R5_G5_B5_A1_uNorm
    {cTextureCompSel_R, cTextureCompSel_0, cTextureCompSel_0, cTextureCompSel_1},  // R32_uInt
    {cTextureCompSel_R, cTextureCompSel_0, cTextureCompSel_0, cTextureCompSel_1},  // R32_sInt
    {cTextureCompSel_R, cTextureCompSel_0, cTextureCompSel_0, cTextureCompSel_1},  // R32_float
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_0, cTextureCompSel_1},  // R16_G16_uNorm
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_0, cTextureCompSel_1},  // R16_G16_uInt
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_0, cTextureCompSel_1},  // R16_G16_sNorm
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_0, cTextureCompSel_1},  // R16_G16_sInt
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_0, cTextureCompSel_1},  // R16_G16_float
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B, cTextureCompSel_1},  // R11_G11_B10_float
    {cTextureCompSel_A, cTextureCompSel_B, cTextureCompSel_G, cTextureCompSel_R},  // A2_B10_G10_R10_uNorm
    {cTextureCompSel_A, cTextureCompSel_B, cTextureCompSel_G, cTextureCompSel_R},  // A2_B10_G10_R10_uInt
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B, cTextureCompSel_A},  // R8_G8_B8_A8_uNorm
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B, cTextureCompSel_A},  // R8_G8_B8_A8_uInt
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B, cTextureCompSel_A},  // R8_G8_B8_A8_sNorm
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B, cTextureCompSel_A},  // R8_G8_B8_A8_sInt
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B, cTextureCompSel_A},  // R8_G8_B8_A8_SRGB
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B, cTextureCompSel_A},  // R10_G10_B10_A2_uNorm
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B, cTextureCompSel_A},  // R10_G10_B10_A2_uInt
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_0, cTextureCompSel_1},  // R32_G32_uInt
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_0, cTextureCompSel_1},  // R32_G32_sInt
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_0, cTextureCompSel_1},  // R32_G32_float
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B, cTextureCompSel_A},  // R16_G16_B16_A16_uNorm
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B, cTextureCompSel_A},  // R16_G16_B16_A16_uInt
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B, cTextureCompSel_A},  // R16_G16_B16_A16_sNorm
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B, cTextureCompSel_A},  // R16_G16_B16_A16_sInt
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B, cTextureCompSel_A},  // R16_G16_B16_A16_float
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B, cTextureCompSel_A},  // R32_G32_B32_A32_uInt
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B, cTextureCompSel_A},  // R32_G32_B32_A32_sInt
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B, cTextureCompSel_A},  // R32_G32_B32_A32_float
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B, cTextureCompSel_A},  // BC1_uNorm
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B, cTextureCompSel_A},  // BC1_SRGB
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B, cTextureCompSel_A},  // BC2_uNorm
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B, cTextureCompSel_A},  // BC2_SRGB
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B, cTextureCompSel_A},  // BC3_uNorm
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B, cTextureCompSel_A},  // BC3_SRGB
    {cTextureCompSel_R, cTextureCompSel_0, cTextureCompSel_0, cTextureCompSel_1},  // BC4_uNorm
    {cTextureCompSel_R, cTextureCompSel_0, cTextureCompSel_0, cTextureCompSel_1},  // BC4_sNorm
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_0, cTextureCompSel_1},  // BC5_uNorm
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_0, cTextureCompSel_1},  // BC5_sNorm
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B, cTextureCompSel_A},  // BC7_uNorm
    {cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B, cTextureCompSel_A},  // BC7_SRGB
    {cTextureCompSel_R, cTextureCompSel_0, cTextureCompSel_0, cTextureCompSel_1},  // Depth_16
    {cTextureCompSel_R, cTextureCompSel_0, cTextureCompSel_0, cTextureCompSel_1},  // Depth_32
    {cTextureCompSel_R, cTextureCompSel_R, cTextureCompSel_R, cTextureCompSel_1},  // Depth_24_uNorm_Stencil_8
    {cTextureCompSel_R, cTextureCompSel_R, cTextureCompSel_R, cTextureCompSel_1},  // Depth_32_float_Stencil_8
};

const FormatInfo cFormatInfo[] = {
    {{0, 0, 0, 0}, {-1, -1, -1, -1}, 0, 0, false, false, false, false, false, false, false},  // Invalid
    {{8, 0, 0, 0}, {0, -1, -1, -1}, 1, 1, false, true, false, true, true, false, false},  // R8_uNorm
    {{8, 0, 0, 0}, {0, -1, -1, -1}, 1, 1, false, false, false, true, true, false, false},  // R8_uInt
    {{8, 0, 0, 0}, {0, -1, -1, -1}, 1, 1, false, true, false, false, true, false, false},  // R8_sNorm
    {{8, 0, 0, 0}, {0, -1, -1, -1}, 1, 1, false, false, false, false, true, false, false},  // R8_sInt
    {{16, 0, 0, 0}, {0, -1, -1, -1}, 2, 1, false, true, false, true, true, false, false},  // R16_uNorm
    {{16, 0, 0, 0}, {0, -1, -1, -1}, 2, 1, false, false, false, true, true, false, false},  // R16_uInt
    {{16, 0, 0, 0}, {0, -1, -1, -1}, 2, 1, false, true, false, false, true, false, false},  // R16_sNorm
    {{16, 0, 0, 0}, {0, -1, -1, -1}, 2, 1, false, false, false, false, true, false, false},  // R16_sInt
    {{16, 0, 0, 0}, {0, -1, -1, -1}, 2, 1, false, false, true, false, true, false, false},  // R16_float
    {{8, 8, 0, 0}, {1, 0, -1, -1}, 2, 2, false, true, false, true, true, false, false},  // R8_G8_uNorm
    {{8, 8, 0, 0}, {1, 0, -1, -1}, 2, 2, false, false, false, true, true, false, false},  // R8_G8_uInt
    {{8, 8, 0, 0}, {1, 0, -1, -1}, 2, 2, false, true, false, false, true, false, false},  // R8_G8_sNorm
    {{8, 8, 0, 0}, {1, 0, -1, -1}, 2, 2, false, false, false, false, true, false, false},  // R8_G8_sInt
    {{5, 6, 5, 0}, {2, 1, 0, -1}, 2, 3, false, true, false, true, false, false, false},  // R5_G6_B5_uNorm
    {{5, 5, 5, 1}, {0, 1, 2, 3}, 2, 4, false, true, false, true, false, false, false},  // A1_B5_G5_R5_uNorm
    {{4, 4, 4, 4}, {3, 2, 1, 0}, 2, 4, false, true, false, true, false, false, false},  // R4_G4_B4_A4_uNorm
    {{5, 5, 5, 1}, {3, 2, 1, 0}, 2, 4, false, true, false, true, false, false, false},  // R5_G5_B5_A1_uNorm
    {{32, 0, 0, 0}, {0, -1, -1, -1}, 4, 1, false, false, false, true, true, false, false},  // R32_uInt
    {{32, 0, 0, 0}, {0, -1, -1, -1}, 4, 1, false, false, false, false, true, false, false},  // R32_sInt
    {{32, 0, 0, 0}, {0, -1, -1, -1}, 4, 1, false, false, true, false, true, false, false},  // R32_float
    {{16, 16, 0, 0}, {1, 0, -1, -1}, 4, 2, false, true, false, true, true, false, false},  // R16_G16_uNorm
    {{16, 16, 0, 0}, {1, 0, -1, -1}, 4, 2, false, false, false, true, true, false, false},  // R16_G16_uInt
    {{16, 16, 0, 0}, {1, 0, -1, -1}, 4, 2, false, true, false, false, true, false, false},  // R16_G16_sNorm
    {{16, 16, 0, 0}, {1, 0, -1, -1}, 4, 2, false, false, false, false, true, false, false},  // R16_G16_sInt
    {{16, 16, 0, 0}, {1, 0, -1, -1}, 4, 2, false, false, true, false, true, false, false},  // R16_G16_float
    {{11, 11, 10, 0}, {2, 1, 0, -1}, 4, 3, false, false, true, true, true, false, false},  // R11_G11_B10_float
    {{10, 10, 10, 2}, {0, 1, 2, 3}, 4, 4, false, true, false, true, true, false, false},  // A2_B10_G10_R10_uNorm
    {{10, 10, 10, 2}, {0, 1, 2, 3}, 4, 4, false, false, false, true, true, false, false},  // A2_B10_G10_R10_uInt
    {{8, 8, 8, 8}, {3, 2, 1, 0}, 4, 4, false, true, false, true, true, false, false},  // R8_G8_B8_A8_uNorm
    {{8, 8, 8, 8}, {3, 2, 1, 0}, 4, 4, false, false, false, true, true, false, false},  // R8_G8_B8_A8_uInt
    {{8, 8, 8, 8}, {3, 2, 1, 0}, 4, 4, false, true, false, false, true, false, false},  // R8_G8_B8_A8_sNorm
    {{8, 8, 8, 8}, {3, 2, 1, 0}, 4, 4, false, false, false, false, true, false, false},  // R8_G8_B8_A8_sInt
    {{8, 8, 8, 8}, {3, 2, 1, 0}, 4, 4, false, true, false, true, true, false, true},  // R8_G8_B8_A8_SRGB
    {{10, 10, 10, 2}, {3, 2, 1, 0}, 4, 4, false, true, false, true, true, false, false},  // R10_G10_B10_A2_uNorm
    {{10, 10, 10, 2}, {3, 2, 1, 0}, 4, 4, false, false, false, true, true, false, false},  // R10_G10_B10_A2_uInt
    {{32, 32, 0, 0}, {0, 1, -1, -1}, 8, 2, false, false, false, true, true, false, false},  // R32_G32_uInt
    {{32, 32, 0, 0}, {0, 1, -1, -1}, 8, 2, false, false, false, false, true, false, false},  // R32_G32_sInt
    {{32, 32, 0, 0}, {0, 1, -1, -1}, 8, 2, false, false, true, false, true, false, false},  // R32_G32_float
    {{16, 16, 16, 16}, {1, 0, 3, 2}, 8, 4, false, true, false, true, true, false, false},  // R16_G16_B16_A16_uNorm
    {{16, 16, 16, 16}, {1, 0, 3, 2}, 8, 4, false, false, false, true, true, false, false},  // R16_G16_B16_A16_uInt
    {{16, 16, 16, 16}, {1, 0, 3, 2}, 8, 4, false, true, false, false, true, false, false},  // R16_G16_B16_A16_sNorm
    {{16, 16, 16, 16}, {1, 0, 3, 2}, 8, 4, false, false, false, false, true, false, false},  // R16_G16_B16_A16_sInt
    {{16, 16, 16, 16}, {1, 0, 3, 2}, 8, 4, false, false, true, false, true, false, false},  // R16_G16_B16_A16_float
    {{32, 32, 32, 32}, {0, 1, 2, 3}, 16, 4, false, false, false, true, true, false, false},  // R32_G32_B32_A32_uInt
    {{32, 32, 32, 32}, {0, 1, 2, 3}, 16, 4, false, false, false, false, true, false, false},  // R32_G32_B32_A32_sInt
    {{32, 32, 32, 32}, {0, 1, 2, 3}, 16, 4, false, false, true, false, true, false, false},  // R32_G32_B32_A32_float
    {{0, 0, 0, 0}, {3, 2, 1, 0}, 8, 4, true, true, false, true, false, false, false},  // BC1_uNorm
    {{0, 0, 0, 0}, {3, 2, 1, 0}, 8, 4, true, true, false, true, false, false, true},  // BC1_SRGB
    {{0, 0, 0, 0}, {3, 2, 1, 0}, 16, 4, true, true, false, true, false, false, false},  // BC2_uNorm
    {{0, 0, 0, 0}, {3, 2, 1, 0}, 16, 4, true, true, false, true, false, false, true},  // BC2_SRGB
    {{0, 0, 0, 0}, {3, 2, 1, 0}, 16, 4, true, true, false, true, false, false, false},  // BC3_uNorm
    {{0, 0, 0, 0}, {3, 2, 1, 0}, 16, 4, true, true, false, true, false, false, true},  // BC3_SRGB
    {{0, 0, 0, 0}, {0, -1, -1, -1}, 8, 1, true, true, false, true, false, false, false},  // BC4_uNorm
    {{0, 0, 0, 0}, {0, -1, -1, -1}, 8, 1, true, true, false, false, false, false, false},  // BC4_sNorm
    {{0, 0, 0, 0}, {1, 0, -1, -1}, 16, 2, true, true, false, true, false, false, false},  // BC5_uNorm
    {{0, 0, 0, 0}, {1, 0, -1, -1}, 16, 2, true, true, false, false, false, false, false},  // BC5_sNorm
    {{0, 0, 0, 0}, {3, 2, 1, 0}, 16, 4, true, true, false, true, false, false, false},  // BC7_uNorm
    {{0, 0, 0, 0}, {3, 2, 1, 0}, 16, 4, true, true, false, true, false, false, true},  // BC7_SRGB
    {{16, 0, 0, 0}, {0, -1, -1, -1}, 2, 1, false, true, false, true, false, true, false},  // Depth_16
    {{32, 0, 0, 0}, {0, -1, -1, -1}, 4, 1, false, false, true, false, false, true, false},  // Depth_32
    {{24, 8, 0, 0}, {0, 1, -1, -1}, 4, 2, false, true, false, true, false, true, false},  // Depth_24_uNorm_Stencil_8
    {{32, 8, 0, 0}, {0, 1, -1, -1}, 8, 2, false, false, true, true, false, true, false},  // Depth_32_float_Stencil_8
};

const char* const cFormatString[] = {
    "cTextureFormat_Invalid",
    "cTextureFormat_R8_uNorm",
    "cTextureFormat_R8_uInt",
    "cTextureFormat_R8_sNorm",
    "cTextureFormat_R8_sInt",
    "cTextureFormat_R16_uNorm",
    "cTextureFormat_R16_uInt",
    "cTextureFormat_R16_sNorm",
    "cTextureFormat_R16_sInt",
    "cTextureFormat_R16_float",
    "cTextureFormat_R8_G8_uNorm",
    "cTextureFormat_R8_G8_uInt",
    "cTextureFormat_R8_G8_sNorm",
    "cTextureFormat_R8_G8_sInt",
    "cTextureFormat_R5_G6_B5_uNorm",
    "cTextureFormat_A1_B5_G5_R5_uNorm",
    "cTextureFormat_R4_G4_B4_A4_uNorm",
    "cTextureFormat_R5_G5_B5_A1_uNorm",
    "cTextureFormat_R32_uInt",
    "cTextureFormat_R32_sInt",
    "cTextureFormat_R32_float",
    "cTextureFormat_R16_G16_uNorm",
    "cTextureFormat_R16_G16_uInt",
    "cTextureFormat_R16_G16_sNorm",
    "cTextureFormat_R16_G16_sInt",
    "cTextureFormat_R16_G16_float",
    "cTextureFormat_R11_G11_B10_float",
    "cTextureFormat_A2_B10_G10_R10_uNorm",
    "cTextureFormat_A2_B10_G10_R10_uInt",
    "cTextureFormat_R8_G8_B8_A8_uNorm",
    "cTextureFormat_R8_G8_B8_A8_uInt",
    "cTextureFormat_R8_G8_B8_A8_sNorm",
    "cTextureFormat_R8_G8_B8_A8_sInt",
    "cTextureFormat_R8_G8_B8_A8_SRGB",
    "cTextureFormat_R10_G10_B10_A2_uNorm",
    "cTextureFormat_R10_G10_B10_A2_uInt",
    "cTextureFormat_R32_G32_uInt",
    "cTextureFormat_R32_G32_sInt",
    "cTextureFormat_R32_G32_float",
    "cTextureFormat_R16_G16_B16_A16_uNorm",
    "cTextureFormat_R16_G16_B16_A16_uInt",
    "cTextureFormat_R16_G16_B16_A16_sNorm",
    "cTextureFormat_R16_G16_B16_A16_sInt",
    "cTextureFormat_R16_G16_B16_A16_float",
    "cTextureFormat_R32_G32_B32_A32_uInt",
    "cTextureFormat_R32_G32_B32_A32_sInt",
    "cTextureFormat_R32_G32_B32_A32_float",
    "cTextureFormat_BC1_uNorm",
    "cTextureFormat_BC1_SRGB",
    "cTextureFormat_BC2_uNorm",
    "cTextureFormat_BC2_SRGB",
    "cTextureFormat_BC3_uNorm",
    "cTextureFormat_BC3_SRGB",
    "cTextureFormat_BC4_uNorm",
    "cTextureFormat_BC4_sNorm",
    "cTextureFormat_BC5_uNorm",
    "cTextureFormat_BC5_sNorm",
    "cTextureFormat_BC7_uNorm",
    "cTextureFormat_BC7_SRGB",
    "cTextureFormat_Depth_16",
    "cTextureFormat_Depth_32",
    "cTextureFormat_Depth_24_uNorm_Stencil_8",
    "cTextureFormat_Depth_32_float_Stencil_8",
};

// clang-format on

const FormatInfo& getInfo(TextureFormat format)
{
    return cFormatInfo[static_cast<u32>(format)];
}

}  // namespace

/**
 * Gets the size of one pixel (or one compressed block).
 * @param format texture format
 * @return the size in bytes
 */
u8 TextureFormatInfo::getPixelByteSize(TextureFormat format)
{
    return getInfo(format).mPixelByteSize;
}

/**
 * Gets the number of color components.
 * @param format texture format
 * @return the component count
 */
u8 TextureFormatInfo::getComponentNum(TextureFormat format)
{
    return getInfo(format).mComponentNum;
}

/**
 * Gets the bit size of one component.
 * @param format texture format
 * @param component component index
 * @return the bit size
 */
u8 TextureFormatInfo::getComponentBitSize(TextureFormat format, s32 component)
{
    return getInfo(format).mComponentBitSize[component];
}

/**
 * Gets the memory order of one component.
 * @param format texture format
 * @param component component index
 * @return the component order
 */
u8 TextureFormatInfo::getComponentOrder(TextureFormat format, s32 component)
{
    return getInfo(format).mComponentOrder[component];
}

/**
 * Checks whether the format is block compressed.
 * @param format texture format
 * @return true if compressed
 */
bool TextureFormatInfo::isCompressed(TextureFormat format)
{
    return getInfo(format).mIsCompressed;
}

/**
 * Checks whether render target compression can be used with the format.
 * @param format texture format
 * @return true if available
 */
bool TextureFormatInfo::isRenderTargetCompressAvailable(TextureFormat format)
{
    const bool is_small = format >= TextureFormat::cTextureFormat_R8_uNorm &&
                          format <= TextureFormat::cTextureFormat_R5_G5_B5_A1_uNorm;
    const bool is_compressed = format >= TextureFormat::cTextureFormat_BC1_uNorm &&
                               format <= TextureFormat::cTextureFormat_BC7_SRGB;
    return !is_compressed & !is_small;
}

/**
 * Checks whether the format stores normalized values.
 * @param format texture format
 * @return true if normalized
 */
bool TextureFormatInfo::isNormalized(TextureFormat format)
{
    return getInfo(format).mIsNormalized;
}

/**
 * Checks whether the format stores floating point values.
 * @param format texture format
 * @return true if floating point
 */
bool TextureFormatInfo::isFloat(TextureFormat format)
{
    return getInfo(format).mIsFloat;
}

/**
 * Checks whether the format stores unsigned values.
 * @param format texture format
 * @return true if unsigned
 */
bool TextureFormatInfo::isUnsigned(TextureFormat format)
{
    return getInfo(format).mIsUnsigned;
}

/**
 * Checks whether the format is an sRGB format.
 * @param format texture format
 * @return true if sRGB
 */
bool TextureFormatInfo::isSRGB(TextureFormat format)
{
    return getInfo(format).mIsSRGB;
}

/**
 * Gets the name of the format.
 * @param format texture format
 * @return the enumerator name
 */
const char* TextureFormatInfo::getString(TextureFormat format)
{
    return cFormatString[static_cast<u32>(format)];
}

/**
 * Checks whether the format can be used for a color render target.
 * @param format texture format
 * @return true if usable
 */
bool TextureFormatInfo::isUsableAsRenderTargetColor(TextureFormat format)
{
    return getInfo(format).mIsUsableAsRenderTargetColor;
}

/**
 * Checks whether the format can be used for a depth render target.
 * @param format texture format
 * @return true if usable
 */
bool TextureFormatInfo::isUsableAsRenderTargetDepth(TextureFormat format)
{
    return getInfo(format).mIsUsableAsRenderTargetDepth;
}

/**
 * Checks whether the format can be used for a depth stencil render target.
 * @param format texture format
 * @return true if usable
 */
bool TextureFormatInfo::isUsableAsRenderTargetDepthStencil(TextureFormat format)
{
    return format == TextureFormat::cTextureFormat_Depth_24_uNorm_Stencil_8 ||
           format == TextureFormat::cTextureFormat_Depth_32_float_Stencil_8;
}

/**
 * Gets the default component selector of the format.
 * @param format texture format
 * @param component component index
 * @return the component selector
 */
TextureCompSel TextureFormatInfo::getDefaultCompSel(TextureFormat format, s32 component)
{
    return cDefaultCompSel[static_cast<u32>(format)][component];
}

/**
 * Converts an NVN texture format to the matching agl texture format.
 * @param format NVN texture format
 * @return the agl texture format
 */
TextureFormat TextureFormatInfo::convFormatDriverToAGL(NVNformat format)
{
    return detail::TextureDataUtil::convFormatDriverToAGL(format);
}

/**
 * Converts an agl texture format to the matching NVN texture format.
 * @param format agl texture format
 * @return the NVN texture format
 */
NVNformat TextureFormatInfo::convFormatAGLToDriver(TextureFormat format)
{
    return detail::TextureDataUtil::convFormatAGLToDriver(format);
}

}  // namespace agl

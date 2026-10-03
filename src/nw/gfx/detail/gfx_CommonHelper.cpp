#include <nn/gfx/detail/gfx_CommonHelper.h>

#include <nn/gfx/gfx_ResShaderData.h>
#include <nn/util.h>

namespace nn::gfx::detail {

/**
 * Embeds the nn::gfx middleware version string into the binary.
 */
void UseMiddleWare() {
    nn::util::ReferSymbol("SDK MW+Nintendo+NintendoSDK_gfx-10_4_0-Release");
}

/**
 * Returns the width in pixels of one compression block of a format.
 *
 * @param format Channel format.
 * @return Block width (4 for uncompressed and BCn/ETC formats).
 */
int GetBlockWidth(ChannelFormat format) {
    switch (format) {
    case ChannelFormat_Pvrtc1_2Bpp:
    case ChannelFormat_Pvrtc1_Alpha_2Bpp:
        return 16;
    case ChannelFormat_Pvrtc1_4Bpp:
    case ChannelFormat_Pvrtc1_Alpha_4Bpp:
    case ChannelFormat_Pvrtc2_Alpha_2Bpp:
    case ChannelFormat_Astc_8x5:
    case ChannelFormat_Astc_8x6:
    case ChannelFormat_Astc_8x8:
        return 8;
    case ChannelFormat_Pvrtc2_Alpha_4Bpp:
    case ChannelFormat_Astc_4x4:
        return 4;
    case ChannelFormat_Astc_5x4:
    case ChannelFormat_Astc_5x5:
        return 5;
    case ChannelFormat_Astc_6x5:
    case ChannelFormat_Astc_6x6:
        return 6;
    case ChannelFormat_Astc_10x5:
    case ChannelFormat_Astc_10x6:
    case ChannelFormat_Astc_10x8:
    case ChannelFormat_Astc_10x10:
        return 10;
    case ChannelFormat_Astc_12x10:
    case ChannelFormat_Astc_12x12:
        return 12;
    default:
        return 4;
    }
}

/**
 * Returns the height in pixels of one compression block of a format.
 *
 * @param format Channel format.
 * @return Block height (4 for uncompressed and BCn/ETC formats).
 */
int GetBlockHeight(ChannelFormat format) {
    switch (format) {
    case ChannelFormat_Pvrtc1_2Bpp:
    case ChannelFormat_Pvrtc1_4Bpp:
    case ChannelFormat_Pvrtc1_Alpha_2Bpp:
    case ChannelFormat_Pvrtc1_Alpha_4Bpp:
    case ChannelFormat_Astc_8x8:
    case ChannelFormat_Astc_10x8:
        return 8;
    case ChannelFormat_Pvrtc2_Alpha_2Bpp:
    case ChannelFormat_Pvrtc2_Alpha_4Bpp:
    case ChannelFormat_Astc_4x4:
    case ChannelFormat_Astc_5x4:
        return 4;
    case ChannelFormat_Astc_5x5:
    case ChannelFormat_Astc_6x5:
    case ChannelFormat_Astc_8x5:
    case ChannelFormat_Astc_10x5:
        return 5;
    case ChannelFormat_Astc_6x6:
    case ChannelFormat_Astc_8x6:
    case ChannelFormat_Astc_10x6:
        return 6;
    case ChannelFormat_Astc_10x10:
    case ChannelFormat_Astc_12x10:
        return 10;
    case ChannelFormat_Astc_12x12:
        return 12;
    default:
        return 4;
    }
}

/**
 * Checks whether a format is a block-compressed format.
 *
 * @param format Channel format.
 * @return true for BCn, EAC, ETC, PVRTC and ASTC formats.
 */
bool IsCompressedFormat(ChannelFormat format) {
    switch (format) {
    case ChannelFormat_Bc1:
    case ChannelFormat_Bc2:
    case ChannelFormat_Bc3:
    case ChannelFormat_Bc4:
    case ChannelFormat_Bc5:
    case ChannelFormat_Bc6:
    case ChannelFormat_Bc7:
    case ChannelFormat_Eac_R11:
    case ChannelFormat_Eac_R11_G11:
    case ChannelFormat_Etc1:
    case ChannelFormat_Etc2:
    case ChannelFormat_Etc2_Mask:
    case ChannelFormat_Etc2_Alpha:
    case ChannelFormat_Pvrtc1_2Bpp:
    case ChannelFormat_Pvrtc1_4Bpp:
    case ChannelFormat_Pvrtc1_Alpha_2Bpp:
    case ChannelFormat_Pvrtc1_Alpha_4Bpp:
    case ChannelFormat_Pvrtc2_Alpha_2Bpp:
    case ChannelFormat_Pvrtc2_Alpha_4Bpp:
    case ChannelFormat_Astc_4x4:
    case ChannelFormat_Astc_5x4:
    case ChannelFormat_Astc_5x5:
    case ChannelFormat_Astc_6x5:
    case ChannelFormat_Astc_6x6:
    case ChannelFormat_Astc_8x5:
    case ChannelFormat_Astc_8x6:
    case ChannelFormat_Astc_8x8:
    case ChannelFormat_Astc_10x5:
    case ChannelFormat_Astc_10x6:
    case ChannelFormat_Astc_10x8:
    case ChannelFormat_Astc_10x10:
    case ChannelFormat_Astc_12x10:
    case ChannelFormat_Astc_12x12:
        return true;
    default:
        return false;
    }
}

/**
 * Checks whether a type format is an sRGB format.
 *
 * @param format Type format.
 * @return true if the format is TypeFormat_UnormSrgb.
 */
bool IsSrgbFormat(TypeFormat format) {
    return format == TypeFormat_UnormSrgb;
}

/**
 * Returns the size in bytes of one pixel, or of one block for compressed formats.
 *
 * @param format Channel format.
 * @return Bytes per pixel (or per block).
 */
int GetBytePerPixel(ChannelFormat format) {
    switch (format) {
    case ChannelFormat_Bc1:
    case ChannelFormat_Bc4:
    case ChannelFormat_Eac_R11:
    case ChannelFormat_Etc1:
    case ChannelFormat_Etc2:
    case ChannelFormat_Etc2_Mask:
    case ChannelFormat_Pvrtc2_Alpha_2Bpp:
    case ChannelFormat_Pvrtc2_Alpha_4Bpp:
        return 8;
    case ChannelFormat_Bc2:
    case ChannelFormat_Bc3:
    case ChannelFormat_Bc5:
    case ChannelFormat_Bc6:
    case ChannelFormat_Bc7:
    case ChannelFormat_Eac_R11_G11:
    case ChannelFormat_Etc2_Alpha:
    case ChannelFormat_Astc_4x4:
    case ChannelFormat_Astc_5x4:
    case ChannelFormat_Astc_5x5:
    case ChannelFormat_Astc_6x5:
    case ChannelFormat_Astc_6x6:
    case ChannelFormat_Astc_8x5:
    case ChannelFormat_Astc_8x6:
    case ChannelFormat_Astc_8x8:
    case ChannelFormat_Astc_10x5:
    case ChannelFormat_Astc_10x6:
    case ChannelFormat_Astc_10x8:
    case ChannelFormat_Astc_10x10:
    case ChannelFormat_Astc_12x10:
    case ChannelFormat_Astc_12x12:
        return 16;
    case ChannelFormat_Pvrtc1_2Bpp:
    case ChannelFormat_Pvrtc1_4Bpp:
    case ChannelFormat_Pvrtc1_Alpha_2Bpp:
    case ChannelFormat_Pvrtc1_Alpha_4Bpp:
        return 32;
    case ChannelFormat_B5_G5_R5_A1:
        return 2;
    default:
        break;
    }

    if (format <= ChannelFormat_R8) {
        return 1;
    } else if (format <= ChannelFormat_R16) {
        return 2;
    } else if (format <= ChannelFormat_R32) {
        return 4;
    } else if (format <= ChannelFormat_R32_G32) {
        return 8;
    } else if (format <= ChannelFormat_R32_G32_B32) {
        return 12;
    } else {
        return 16;
    }
}

/**
 * Calculates the size in bytes of one image (one mip level of one array layer).
 *
 * @param format Channel format.
 * @param width Width in pixels.
 * @param height Height in pixels.
 * @param depth Depth in pixels.
 * @return Image size in bytes.
 */
size_t CalculateImageSize(ChannelFormat format, uint32_t width, uint32_t height, uint32_t depth) {
    if (IsCompressedFormat(format)) {
        int blockWidth = GetBlockWidth(format);
        int blockHeight = GetBlockHeight(format);
        width = (width + blockWidth - 1) / blockWidth;
        height = (height + blockHeight - 1) / blockHeight;
    }

    return width * height * depth * GetBytePerPixel(format);
}

/**
 * Returns the number of channels of a format.
 *
 * @param format Channel format.
 * @return Channel count.
 */
int GetChannelCount(ChannelFormat format) {
    const int s_ChannelCountTable[] = {
        0, 2, 1, 4, 4, 4, 4, 3, 3, 2, 1, 4, 4, 4, 4, 3, 3, 3, 2, 2, 1, 4, 2, 2, 3, 4, 4, 4, 4, 1,
        2, 3, 4, 1, 2, 3, 3, 4, 4, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    };

    return s_ChannelCountTable[format];
}

/**
 * Calculates the size in bytes of one row of pixels (or of blocks for compressed formats).
 *
 * @param width Width in pixels.
 * @param format Channel format.
 * @return Row size in bytes.
 */
size_t CalculateRowSize(uint32_t width, ChannelFormat format) {
    if (IsCompressedFormat(format)) {
        int blockWidth = GetBlockWidth(format);
        width = (width + blockWidth - 1) / blockWidth;
    }

    return GetBytePerPixel(format) * width;
}

/**
 * Checks that a memory pool property has exactly one valid CPU and one valid GPU page property.
 *
 * @param value MemoryPoolProperty bit combination.
 * @return true if the CPU and GPU page properties are each one of the valid values.
 */
bool IsValidMemoryPoolProperty(int value) {
    int cpuPageProperty = value & 0x7;
    int gpuPageProperty = value & 0x38;

    return (cpuPageProperty == MemoryPoolProperty_CpuInvisible ||
            cpuPageProperty == MemoryPoolProperty_CpuUncached ||
            cpuPageProperty == MemoryPoolProperty_CpuCached) &&
           (gpuPageProperty == MemoryPoolProperty_GpuInvisible ||
            gpuPageProperty == MemoryPoolProperty_GpuUncached ||
            gpuPageProperty == MemoryPoolProperty_GpuCached);
}

/**
 * Derives the image dimension of a view from the storage dimension of its texture.
 *
 * @param imageStorageDimension Storage dimension of the texture.
 * @param isArray Whether the texture is an array texture.
 * @param isMultisample Whether the texture is multisampled.
 * @return Matching image dimension.
 */
ImageDimension GetImageDimension(ImageStorageDimension imageStorageDimension, bool isArray,
                                 bool isMultisample) {
    ImageDimension ret;

    switch (imageStorageDimension) {
    case ImageStorageDimension_1d:
        if (isArray) {
            ret = ImageDimension_1dArray;
        } else {
            ret = ImageDimension_1d;
        }

        break;
    case ImageStorageDimension_2d:
        if (isArray) {
            if (isMultisample) {
                ret = ImageDimension_2dMultisampleArray;
            } else {
                ret = ImageDimension_2dArray;
            }
        } else {
            if (isMultisample) {
                ret = ImageDimension_2dMultisample;
            } else {
                ret = ImageDimension_2d;
            }
        }

        break;
    case ImageStorageDimension_3d:
        ret = ImageDimension_3d;
        break;
    default:
        NN_UNEXPECTED_DEFAULT;
    }

    return ret;
}

/**
 * Checks whether a shader container was built for the given low-level API.
 *
 * @param rResShaderContainer Shader container to check.
 * @param lowLevelApi Low-level API type.
 * @param apiVersion Low-level API version (unused).
 * @return true if the container targets lowLevelApi.
 */
bool CheckBinaryTarget(const ResShaderContainerData& rResShaderContainer, int lowLevelApi,
                       int apiVersion) {
    return rResShaderContainer.targetApiType == lowLevelApi;
}

}  // namespace nn::gfx::detail

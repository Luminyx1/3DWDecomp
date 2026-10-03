#pragma once

#include <nn/gfx/gfx_Enum.h>
#include <nn/types.h>

namespace nn::gfx {

struct ResShaderContainerData;

namespace detail {

void UseMiddleWare();

int GetBlockWidth(ChannelFormat format);
int GetBlockHeight(ChannelFormat format);
bool IsCompressedFormat(ChannelFormat format);
bool IsSrgbFormat(TypeFormat format);
int GetBytePerPixel(ChannelFormat format);
size_t CalculateImageSize(ChannelFormat format, uint32_t width, uint32_t height, uint32_t depth);
int GetChannelCount(ChannelFormat format);
size_t CalculateRowSize(uint32_t width, ChannelFormat format);
bool IsValidMemoryPoolProperty(int value);
ImageDimension GetImageDimension(ImageStorageDimension imageStorageDimension, bool isArray,
                                 bool isMultisample);
bool CheckBinaryTarget(const ResShaderContainerData& rResShaderContainer, int lowLevelApi,
                       int apiVersion);

}  // namespace detail
}  // namespace nn::gfx

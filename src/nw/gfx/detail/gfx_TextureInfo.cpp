#include <nn/gfx/gfx_TextureInfo.h>
namespace nn::gfx {
void TextureInfo::SetDefault() {
    imageStorageDimension = ImageStorageDimension_2d;
    imageFormat = 0; gpuAccessFlags = 0; tileMode = 0;
    width = 1; height = 1; depth = 1; arrayLength = 0;
    swizzle = 0; mipCount = 1; multisampleCount = 1;
}

void TextureViewInfo::SetDefault() {
    imageDimension = ImageDimension_2d;
    depthStencilTextureMode = 0;
    imageFormat = 0;
    channelMapping[0] = 2; channelMapping[1] = 3;
    channelMapping[2] = 4; channelMapping[3] = 5;
    EditSubresourceRange().SetDefault();
    pTexture = nullptr;
}

void TextureMipRange::SetDefault() { minMipLevel = 0; mipCount = 1; }
void TextureArrayRange::SetDefault() { baseArrayIndex = 0; arrayLength = 1; }
void TextureSubresourceRange::SetDefault() { EditMipRange().SetDefault(); EditArrayRange().SetDefault(); }
void ColorTargetViewInfo::SetDefault() {
    imageDimension = ImageDimension_2d; imageFormat = 0; mipLevel = 0;
    EditArrayRange().SetDefault(); pTexture = nullptr;
}

void DepthStencilViewInfo::SetDefault() {
    imageDimension = ImageDimension_2d; mipLevel = 0;
    EditArrayRange().SetDefault(); pTexture = nullptr;
}

void TextureSubresource::SetDefault() { mipLevel = 0; arrayIndex = 0; }
void TextureCopyRegion::SetDefault() {
    offsetU = 0; offsetV = 0; offsetW = 0;
    width = 1; height = 1; depth = 1;
    subresource.mipLevel = 0; subresource.arrayIndex = 0; arrayLength = 1;
}

void BufferTextureCopyRegion::SetDefault() {
    bufferOffset = 0; bufferImageWidth = 0; bufferImageHeight = 0;
    EditTextureCopyRegion().SetDefault();
}
}

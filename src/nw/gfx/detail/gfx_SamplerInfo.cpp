#include <nn/gfx/gfx_SamplerInfo.h>
namespace nn::gfx {
void SamplerInfo::SetDefault() {
    addressU = TextureAddressMode_ClampToEdge;
    addressV = TextureAddressMode_ClampToEdge;
    addressW = TextureAddressMode_ClampToEdge;
    comparisonFunction = ComparisonFunction_Never;
    borderColorType = TextureBorderColorType_White;
    maxAnisotropy = 1;
    filterMode = FilterMode_MinLinear_MagLinear_MipLinear;
    minLod = -1000;
    maxLod = 1000;
    lodBias = 0;
}
}

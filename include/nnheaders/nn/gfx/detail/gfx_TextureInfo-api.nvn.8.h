#pragma once

#include <nn/gfx/gfx_Common.h>
#include <nvn/nvn.h>

namespace nn::gfx::detail {

/**
 * NVN-native texture description: wraps an already configured NVNtextureBuilder.
 */
template <>
class TextureInfoImpl<ApiVariationNvn8> {
public:
    NVNtextureBuilder nvnTextureBuilder;
};

}  // namespace nn::gfx::detail

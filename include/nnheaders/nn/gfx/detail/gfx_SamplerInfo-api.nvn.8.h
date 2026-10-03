#pragma once

#include <nn/gfx/gfx_Common.h>
#include <nvn/nvn.h>

namespace nn::gfx::detail {

/**
 * NVN-native sampler description: wraps an already configured NVNsamplerBuilder.
 */
template <>
class SamplerInfoImpl<ApiVariationNvn8> {
public:
    NVNsamplerBuilder nvnSamplerBuilder;
};

}  // namespace nn::gfx::detail

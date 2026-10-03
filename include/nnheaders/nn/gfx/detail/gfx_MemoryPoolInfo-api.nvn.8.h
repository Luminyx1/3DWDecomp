#pragma once

#include <nn/gfx/gfx_Common.h>
#include <nvn/nvn.h>

namespace nn::gfx::detail {

/**
 * NVN-native memory pool description: wraps an already configured NVNmemoryPoolBuilder.
 */
template <>
class MemoryPoolInfoImpl<ApiVariationNvn8> {
public:
    NVNmemoryPoolBuilder nvnMemoryPoolBuilder;
};

}  // namespace nn::gfx::detail

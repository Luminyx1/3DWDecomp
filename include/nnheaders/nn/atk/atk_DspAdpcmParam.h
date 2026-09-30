#pragma once
#include <nn/atk/atk_DecodeAdpcm.h>

namespace nn::atk {
struct DspAdpcmParam {
    audio::AdpcmParameter parameter;
    AdpcmContext context;
};
namespace detail {
struct DspAdpcmLoopParam { AdpcmContext context; };
}
static_assert(sizeof(DspAdpcmParam) == 0x26, "DspAdpcmParam size");
static_assert(sizeof(detail::DspAdpcmLoopParam) == 6, "DspAdpcmLoopParam size");
}

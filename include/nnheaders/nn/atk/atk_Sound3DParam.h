#pragma once
#include <nn/util/util_MathTypes.h>

namespace nn::atk {
struct Sound3DParam {
    Sound3DParam();
    util::Vector3fType position;
    util::Vector3fType velocity;
    u32 flags;
    u32 userParam;
    float decayDistance;
    float decayRatio;
    u8 decayCurve;
    u8 dopplerFactor;
};
static_assert(sizeof(Sound3DParam) == 0x40, "Sound3DParam size");
} // namespace nn::atk

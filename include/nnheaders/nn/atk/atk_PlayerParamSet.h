#pragma once
#include <nn/types.h>

namespace nn::atk::detail::driver {
struct PlayerParamSet {
    void Initialize();
    float volume;
    float pitch;
    float _08;
    float biquadValue;
    s8 biquadType;
    u32 _14;
    u32 _18;
    u32 _1c;
    float _20;
    float _24;
    float _28[12];
    void* _58[3];
};
static_assert(sizeof(PlayerParamSet) == 0x70, "PlayerParamSet size");
}

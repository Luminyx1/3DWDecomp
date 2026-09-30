#pragma once
#include <nn/types.h>
namespace nn::ui2d {
struct ResStateCalculatedVariables {
    u8 easingType;
    u8 _01[3];
    u8 limitMode;
    u8 _05[3];
    float duration, delay, offset, scale, minimum, maximum;
};
struct StateMachineVariable {
    u8 _00[0x2c];
    float defaultValue;
    u8 _30[0x10];
    float value;
};
struct StateMachineCalclatedVariable {
    u8 _00[0x10];
    const ResStateCalculatedVariables* resource;
    u8 _18[0xc];
    float previousValue, nextValue;
    u32 _2C;
    float elapsed;
    bool changed;
};
class StateMachineVariableManager {
public:
    bool ResetToDefalut_(StateMachineVariable* variable);
    float DoCalculateCalcVarLinearScaling_(const ResStateCalculatedVariables* resource, float value, float step);
    float DoCalculateCalcVarRangeLimit_(const ResStateCalculatedVariables* resource, float value, float step);
    void DoUpdateCalcVarOnValueChanged_(StateMachineCalclatedVariable* variable, float previous, float next);
};
}

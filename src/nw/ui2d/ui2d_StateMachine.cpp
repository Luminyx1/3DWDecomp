#include <nn/ui2d/ui2d_StateMachine.h>
#include <cmath>
namespace nn::ui2d {
// variable is restored to its resource default; returns whether its value changed.
bool StateMachineVariableManager::ResetToDefalut_(StateMachineVariable* variable) {
    if (variable->value != variable->defaultValue) { variable->value = variable->defaultValue; return true; }
    return false;
}
// resource supplies scale and offset for value; step is unused for linear scaling.
float StateMachineVariableManager::DoCalculateCalcVarLinearScaling_(const ResStateCalculatedVariables* resource, float value, float step) {
    return value * resource->scale + resource->offset;
}
// resource selects wrapping or clamping for value; step is unused for range limiting.
float StateMachineVariableManager::DoCalculateCalcVarRangeLimit_(const ResStateCalculatedVariables* resource, float value, float step) {
    if (resource->limitMode == 0) return value - resource->maximum * static_cast<int>(std::floor(value / resource->maximum));
    if (resource->limitMode == 1) {
        if (resource->maximum < value) value = resource->maximum;
        if (value < resource->minimum) value = resource->minimum;
    }
    return value;
}
// variable starts a fresh interpolation between previous and next values.
void StateMachineVariableManager::DoUpdateCalcVarOnValueChanged_(StateMachineCalclatedVariable* variable, float previous, float next) {
    variable->previousValue = previous; variable->nextValue = next; variable->elapsed = 0; variable->changed = true;
}
}

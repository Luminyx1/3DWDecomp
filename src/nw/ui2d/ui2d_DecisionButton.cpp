#include <nn/ui2d/ui2d_DecisionButton.h>
namespace nn::ui2d {
// Consume selection changes only once the pending transition permits them.
bool DecisionButton::ProcessOn() {
    bool processed = true;
    switch (mState) {
    case cState_Off:
        ChangeState(cState_OnStart);
        StartOn();
        break;
    case cState_OffStart:
        ChangeState(cState_OnStart);
        StartOn();
        break;
    case cState_CancelStart:
        processed = false;
        break;
    default:
        break;
    }

    return processed;
}

bool DecisionButton::ProcessOff() {
    switch (mState) {
    case cState_OnStart:
        ChangeState(cState_OffStart);
        StartOff();
        break;
    case cState_On:
        ChangeState(cState_OffStart);
        StartOff();
        break;
    default:
        break;
    }

    return true;
}
}

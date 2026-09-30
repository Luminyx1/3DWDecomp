#include <eui/euiTapButton.h>
#include <eui/euiAnimator.h>

namespace eui {
const char* TapButton::getClassName() const { return "TapButton"; }
void TapButton::On() {}
void TapButton::Off() {}

// NON_MATCHING: equivalent state dispatch currently produces a different jump table.
bool TapButton::ProcessDown() {
    bool processed = true;
    switch (mState) {
    case cState_Off:
        StartDown();
        ChangeState(cState_DownStart);
        processed = false;
        break;
    case cState_OnStart:
    case cState_OffStart:
    case cState_On:
    case cState_CancelStart:
        processed = false;
        break;
    }
    return processed;
}

// Deliver the completed press before immediately returning to the idle state.
void TapButton::FinishDown() {
    ChangeState(cState_Down);
    ChangeState(cState_Off);
}

void TapButton::ForceOff() {
    ButtonBase::ForceOff();
    SelectStateAnim(4)->StopAtMin();
}
}

#include <eui/euiDecisionButton.h>

namespace eui {

const char* DecisionButton::getClassName() const { return "DecisionButton"; }

// rOther supplies button properties; pLayout owns the clone; pHeap holds its animations.
DecisionButton::DecisionButton(const DecisionButton& rOther, LayoutEx* pLayout,
                               sead::Heap* pHeap) {
    CloneImpl_(rOther, pLayout, pHeap);
    mFlags |= 0x20;
}

bool DecisionButton::ProcessOn() {
    bool processed = true;

    switch (mState) {
    case cState_Off:
        StartOn();
        ChangeState(cState_OnStart);
        break;
    case cState_OffStart:
        StartOn();
        ChangeState(cState_OnStart);
        break;
    case cState_CancelStart:
        processed = false;
        break;
    }

    return processed;
}

bool DecisionButton::ProcessOff() {
    if (mState == cState_OnStart) {
        StartOff();
        ChangeState(cState_OffStart);
    } else if (mState == cState_On) {
        StartOff();
        ChangeState(cState_OffStart);
    }

    return true;
}

void DecisionButton::FinishDown() { ChangeState(cState_Down); }

}  // namespace eui

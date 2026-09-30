#include <eui/euiButtonBase.h>

namespace eui {

ButtonBase::ButtonBase() : mState(cState_Off), _39(0), mFlags(0x1f), _44(0) {}
const char* ButtonBase::getClassName() const { return "ButtonBase"; }

// action replaces an already queued copy and drops all actions following that copy.
// A new action is appended only when the four-entry queue has room.
void ButtonBase::ActionQueue::PushWithOmit(Action action) {
    for (int i = 0; i < count; ++i) {
        if (actions[i] == action) { count = i + 1; return; }
    }

    if (count < 4) {
        actions[count] = action;
        ++count;
    }
}

// Remove the oldest queued action, preserving the order of the remaining actions.
void ButtonBase::ActionQueue::Pop() {
    if (count > 0) {
        for (int i = 0; i < count - 1; ++i) actions[i] = actions[i + 1];
        --count;
    }
}

bool ButtonBase::ActionQueue::IsDownExist() const {
    for (int i = 0; i < count; ++i)
        if (actions[i] == cAction_Down) return true;
    return false;
}

bool ButtonBase::IsDowning() const {
    return mState == cState_DownStart || mState == cState_Down || mActions.IsDownExist();
}

// Forced transitions discard pending actions before selecting the settled state.
void ButtonBase::ForceOff() { mActions.count = 0; ForceChangeState(cState_Off); }
void ButtonBase::ForceOn() { mActions.count = 0; ForceChangeState(cState_On); }
void ButtonBase::ForceDown() { mActions.count = 0; ForceChangeState(cState_Down); }

// active controls whether this button accepts input actions.
void ButtonBase::SetActive(bool active) {
    if (active) mFlags |= 0x10;
    else mFlags &= ~0x10;
}

bool ButtonBase::UpdateOn() { return true; }
bool ButtonBase::UpdateOff() { return true; }
bool ButtonBase::UpdateDown() { return true; }
bool ButtonBase::UpdateCancel() { return true; }
void ButtonBase::StartOn() {}
void ButtonBase::StartOff() {}
void ButtonBase::StartDown() {}
void ButtonBase::StartCancel() {}
void ButtonBase::FinishOn() { ChangeState(cState_On); }
void ButtonBase::FinishOff() { ChangeState(cState_Off); }
void ButtonBase::FinishDown() { ChangeState(cState_Down); }
void ButtonBase::FinishCancel() { ChangeState(cState_Off); }

// state is the next button state; derived classes may add transition callbacks.
void ButtonBase::ChangeState(State state) { mState = state; }
// state is selected immediately, without starting a transition animation.
void ButtonBase::ForceChangeState(State state) { mState = state; }

bool ButtonBase::ProcessCancel() {
    bool processed = true;
    switch (mState) {
    case cState_DownStart:
        processed = false;
        break;
    case cState_Down:
        StartCancel();
        ChangeState(cState_CancelStart);
        break;
    }

    return processed;
}

bool ButtonBase::ProcessOn() {
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
    case cState_DownStart:
    case cState_CancelStart:
        processed = false;
        break;
    }

    return processed;
}

bool ButtonBase::ProcessOff() {
    bool processed = true;
    switch (mState) {
    case cState_OnStart:
    case cState_On:
    case cState_Down:
        StartOff();
        ChangeState(cState_OffStart);
        break;
    case cState_DownStart:
        processed = false;
        break;
    }

    return processed;
}

bool ButtonBase::ProcessDown() {
    switch (mState) {
    case cState_Off:
    case cState_OffStart:
        StartOn();
        ChangeState(cState_OnStart);
        return false;
    case cState_OnStart:
    case cState_CancelStart:
        return false;
    case cState_On:
        StartDown();
        ChangeState(cState_DownStart);
        break;
    }

    return true;
}

// Consume one queued action only when the state-specific handler accepts it.
void ButtonBase::ProcessActionFromQueue() {
    if (mActions.count == 0) return;
    bool processed = false;
    switch (mActions.actions[0]) {
    case cAction_On: processed = ProcessOn(); break;
    case cAction_Off: processed = ProcessOff(); break;
    case cAction_Down: processed = ProcessDown(); break;
    case cAction_Cancel: processed = ProcessCancel(); break;
    }

    if (processed) mActions.Pop();
}

}  // namespace eui

#include <nn/ui2d/ui2d_ButtonBase.h>

namespace nn::ui2d {

ButtonBase::ButtonBase() : mState(cState_Off), mFlags(0x1f) {}
ButtonBase::~ButtonBase() = default;

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

bool ButtonBase::ActionQueue::HasDownAction() const {
    for (int i = 0; i < count; ++i)
        if (actions[i] == cAction_Down) return true;
    return false;
}

bool ButtonBase::IsDowning() const {
    return mState == cState_DownStart || mState == cState_Down || mActions.HasDownAction();
}

// Forced transitions discard pending actions before selecting the settled state.
void ButtonBase::ForceOff() { mActions.count = 0; ForceChangeState(cState_Off); }
void ButtonBase::ForceOn() { mActions.count = 0; ForceChangeState(cState_On); }
void ButtonBase::ForceDown() { mActions.count = 0; ForceChangeState(cState_Down); }

// active controls whether this button accepts input actions.
void ButtonBase::SetActive(bool active) {
    mFlags = (active ? 0x10u : 0u) | (mFlags & ~0x10u);
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
        ChangeState(cState_CancelStart);
        StartCancel();
        break;
    default: break;
    }

    return processed;
}

bool ButtonBase::ProcessOn() {
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
    case cState_DownStart:
    case cState_CancelStart:
        processed = false;
        break;
    default: break;
    }

    return processed;
}

bool ButtonBase::ProcessOff() {
    bool processed = true;
    switch (mState) {
    case cState_OnStart:
    case cState_On:
    case cState_Down:
        ChangeState(cState_OffStart);
        StartOff();
        break;
    case cState_DownStart:
        processed = false;
        break;
    default: break;
    }

    return processed;
}

bool ButtonBase::ProcessDown() {
    switch (mState) {
    case cState_Off:
    case cState_OffStart:
        ChangeState(cState_OnStart);
        StartOn();
        return false;
    case cState_OnStart:
    case cState_CancelStart:
        return false;
    case cState_On:
        ChangeState(cState_DownStart);
        StartDown();
        break;
    default: break;
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

void ButtonBase::On() { if ((mFlags & 0x11) == 0x11) mActions.PushWithOmit(cAction_On); }
void ButtonBase::Off() { if (mFlags & 2) mActions.PushWithOmit(cAction_Off); }
void ButtonBase::Down() { if ((mFlags & 0x14) == 0x14) mActions.PushWithOmit(cAction_Down); }
void ButtonBase::Cancel() { if (mFlags & 8) mActions.PushWithOmit(cAction_Cancel); }

void ButtonBase::Update() {
    ProcessActionFromQueue();
    switch (mState) {
    case cState_OnStart:
        if (UpdateOn()) { FinishOn(); ProcessActionFromQueue(); }
        break;
    case cState_OffStart:
        if (UpdateOff()) { FinishOff(); ProcessActionFromQueue(); }
        break;
    case cState_DownStart:
        if (UpdateDown()) { FinishDown(); ProcessActionFromQueue(); }
        break;
    case cState_CancelStart:
        if (UpdateCancel()) { FinishCancel(); ProcessActionFromQueue(); }
        break;
    default: break;
    }
}

}  // namespace nn::ui2d

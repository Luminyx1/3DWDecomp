#include <eui/euiSelectButton.h>
#include <eui/euiAnimator.h>
#include <eui/euiAnimatorSet.h>
#include <eui/euiLayoutEx.h>
#include <nn/ui2d/ui2d_ControlSrc.h>

namespace eui {
// rSource names each state animation; pLayout creates and owns the animator set.
void SelectButton::BuildStateAnim(const nn::ui2d::ControlSrc& rSource, LayoutEx* pLayout) {
    const char* names[] = {
        rSource.FindFunctionalAnimName("On"),
        rSource.FindFunctionalAnimName("TouchOn"),
        rSource.FindFunctionalAnimName("Off"),
        rSource.FindFunctionalAnimName("TouchOff"),
        rSource.FindFunctionalAnimName("Decide"),
        rSource.FindFunctionalAnimName("TouchDecide"),
        rSource.FindFunctionalAnimName("Cancel")
    };

    mStateAnimators = pLayout->createAnimatorSet(names, 7, true);
}

const char* SelectButton::getClassName() const { return "SelectButton"; }

// rOther supplies button properties; pLayout owns the clone; pHeap holds its animations.
SelectButton::SelectButton(const SelectButton& rOther, LayoutEx* pLayout, sead::Heap* pHeap) {
    CloneImpl_(rOther, pLayout, pHeap);
}

// rPosition is the pointer position; cancelling buttons temporarily reject hit tests.
bool SelectButton::HitTest(const sead::Vector2f& rPosition) const {
    if (mState == cState_CancelStart) return false;
    return AnimButton::HitTest(rPosition);
}

bool SelectButton::ProcessOn() {
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

bool SelectButton::ProcessOff() {
    if (mState == cState_OnStart) {
        StartOff();
        ChangeState(cState_OffStart);
    } else if (mState == cState_On) {
        StartOff();
        ChangeState(cState_OffStart);
    }

    return true;
}

void SelectButton::FinishDown() { ChangeState(cState_Down); }

void SelectButton::StartCancel() {
    mStateAnimators->select(6)->Play(Animator::cPlayType_OneTime, 1);
}

bool SelectButton::UpdateCancel() { return (mStateAnimators->mSelected->mFlags & 1) != 0; }

void SelectButton::FinishCancel() {
    SelectStateAnim(0)->StopAtMin();
    ChangeState(cState_Off);
}

bool SelectButton::ProcessCancel() {
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

}

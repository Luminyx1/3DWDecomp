#include <nn/ui2d/ui2d_SelectButton.h>
#include <nn/ui2d/ui2d_Animator.h>
#include <nn/ui2d/ui2d_ControlSrc.h>
#include <nn/ui2d/ui2d_Layout.h>
namespace nn::ui2d {
// device creates resources, layout supplies animation targets, and source
// identifies the cancel animation in addition to the standard button animations.
void SelectButton::Build(nn::gfx::Device* device, Layout* layout, const ControlSrc& source) {
    AnimButton::Build(device, layout, source);
    mCancelAnimator = layout->CreateGroupAnimatorAuto(device, source.FindFunctionalAnimName("Cancel"), false);
}

// position is the pointer position in the hit box's coordinate system.
bool SelectButton::IsHit(const nn::util::Float2& position) const {
    if (mState == cState_CancelStart) return false;
    return AnimButton::IsHit(position);
}

bool SelectButton::ProcessOff() {
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

bool SelectButton::ProcessCancel() {
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

void SelectButton::FinishCancel() {
    ChangeState(cState_Off);
    if (mOnAnimator) mOnAnimator->StopAtStartFrame();
}
}

#include <nn/ui2d/ui2d_TouchDragButton.h>
#include <nn/ui2d/ui2d_Animator.h>
#include <nn/ui2d/ui2d_ControlSrc.h>
#include <nn/ui2d/ui2d_Layout.h>
namespace nn::ui2d {
TouchDragButton::TouchDragButton() { mFlags = (mFlags & ~0xc6u) | 0x80u; }
// device creates the animators, layout provides the dragged pane, and source
// names the touch, release, disable, and hit-test resources.
void TouchDragButton::Build(nn::gfx::Device* device, Layout* layout, const ControlSrc& source) {
    mDownAnimator = layout->CreateGroupAnimatorAuto(device, source.FindFunctionalAnimName("Touch"), true);
    mDownAnimator->StopAtStartFrame();
    mCancelAnimator = layout->CreateGroupAnimatorAuto(device, source.FindFunctionalAnimName("Release"), false);
    const char* disable = source.FindFunctionalAnimName("Disable");
    if (disable && *disable) mDisableAnimator = layout->CreateGroupAnimatorAuto(device, disable, true);
    mHitPane = layout->mRootPane->FindPaneByName(source.FindFunctionalPaneName("Hit"), true);
    mName = layout->mRootPane->mParent ? layout->mRootPane->mPanelName : static_cast<const char*>(layout->_30);
    mDragPane = layout->mRootPane;
}

bool TouchDragButton::ProcessOn() {
    switch (mState) {
    case cState_Off:
        ChangeState(cState_DownStart); StartDown(); return true;
    case cState_CancelStart: return false;
    default: return true;
    }
}

bool TouchDragButton::ProcessCancel() {
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

void TouchDragButton::FinishCancel() { ChangeState(cState_Off); }
}

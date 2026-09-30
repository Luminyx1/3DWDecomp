#include <eui/euiTwoTouchCheckKeepButton.h>
#include <eui/euiAnimator.h>
#include <eui/euiAnimatorSet.h>
#include <eui/euiLayoutEx.h>
#include <nn/ui2d/ui2d_ControlSrc.h>
namespace eui {
const char* TwoTouchCheckKeepButton::getClassName() const { return "TwoTouchCheckKeepButton"; }
TwoTouchCheckKeepButton::TwoTouchCheckKeepButton()
    : mFirstTouchAnimators(nullptr), mSecondTouchAnimators(nullptr), mTouched(0) {}
// rOther supplies properties; pLayout owns the clone; pHeap holds copied animations.
TwoTouchCheckKeepButton::TwoTouchCheckKeepButton(const TwoTouchCheckKeepButton& rOther,
                                               LayoutEx* pLayout, sead::Heap* pHeap)
    : CheckKeepButton(rOther, pLayout, pHeap), mFirstTouchAnimators(nullptr),
      mSecondTouchAnimators(nullptr), mTouched(0) {
    mSecondTouchAnimators = new (pHeap, 8) AnimatorSet(*rOther.mSecondTouchAnimators, pLayout, pHeap);
    mSecondTouchAnimators->SetSkipFirstFrameAll(true);
    mSecondTouchAnimators->SetSoundLinkAll(false);
    Animator* pSelected = mSecondTouchAnimators->mSelected;
    pSelected->nn::ui2d::AnimTransform::SetEnabled(false);
    pSelected->mStep = 0;
    mFirstTouchAnimators = mStateAnimators;
}

bool TwoTouchCheckKeepButton::IsTouchOnce() const {
    if (!(mFlags & 0x40)) return true;
    return mTouched != 0;
}

// rPosition is the pointer position; cancellation temporarily rejects hit tests.
bool TwoTouchCheckKeepButton::HitTest(const sead::Vector2f& rPosition) const {
    if (mState == cState_CancelStart) return false;
    return AnimButton::HitTest(rPosition);
}

// touched chooses the first- or second-touch animations and resets the button state.
// NON_MATCHING: the boolean conversion uses an additional saved register.
void TwoTouchCheckKeepButton::ForceSetTouchOnce(bool touched) {
    if (!(mFlags & 0x40)) return;
    Animator* pSelected = mStateAnimators->mSelected;
    pSelected->nn::ui2d::AnimTransform::SetEnabled(false);
    pSelected->mStep = 0;
    AnimatorSet** ppSet = touched ? &mSecondTouchAnimators : &mFirstTouchAnimators;
    mStateAnimators = *ppSet;
    mTouched = touched;
    ForceOff();
}

void TwoTouchCheckKeepButton::ActivateByBoxCursor() {
    if (mFlags & 0x40) {
        mStateAnimators = mFirstTouchAnimators;
        mTouched = 0;
    }

    AnimButton::ActivateByBoxCursor();
}

void TwoTouchCheckKeepButton::InactivateByBoxCursor() {
    AnimButton::InactivateByBoxCursor();

    if ((mFlags & 0x40) && mChecked && mState == cState_DownStart) ForceOff();
}

void TwoTouchCheckKeepButton::StartDown() {
    if (IsTouchOnce()) CheckKeepButton::StartDown();
    else AnimButton::StartDown();
}

bool TwoTouchCheckKeepButton::UpdateDown() {
    if (IsTouchOnce()) return CheckButton::UpdateDown();
    return AnimButton::UpdateDown();
}

void TwoTouchCheckKeepButton::FinishDown() {
    if (!IsTouchOnce()) {
        mStateAnimators = mSecondTouchAnimators;
        mTouched = 1;
    }

    AnimButton::FinishDown();
}

void TwoTouchCheckKeepButton::StartCancel() {
    mStateAnimators->select(6)->Play(Animator::cPlayType_OneTime, 1);
}

bool TwoTouchCheckKeepButton::UpdateCancel() { return (mStateAnimators->mSelected->mFlags & 1) != 0; }
void TwoTouchCheckKeepButton::FinishCancel() {
    mStateAnimators = mFirstTouchAnimators;
    mTouched = 0;
    SelectStateAnim(0)->StopAtMin();
    ChangeState(cState_Off);
}

// rSource names both touch-state animation sets; pLayout creates the corresponding animators.
void TwoTouchCheckKeepButton::BuildStateAnim(const nn::ui2d::ControlSrc& rSource, LayoutEx* pLayout) {
    AnimButton::BuildStateAnim(rSource, pLayout);
    const char* names[] = {
        rSource.FindFunctionalAnimName("TouchOn2"),
        rSource.FindFunctionalAnimName(""),
        rSource.FindFunctionalAnimName("TouchOff2"),
        rSource.FindFunctionalAnimName(""),
        rSource.FindFunctionalAnimName("TouchDecide2"),
        rSource.FindFunctionalAnimName(""),
        rSource.FindFunctionalAnimName("TouchCancel"),
        rSource.FindFunctionalAnimName(""),
    };

    mSecondTouchAnimators = pLayout->createAnimatorSet(names, 8, true);
    mSecondTouchAnimators->SetSkipFirstFrameAll(true);
    mSecondTouchAnimators->SetSoundLinkAll(false);
    Animator* selected = mSecondTouchAnimators->mSelected;
    selected->nn::ui2d::AnimTransform::SetEnabled(false);
    selected->mStep = 0;
    mFirstTouchAnimators = mStateAnimators;
}

bool TwoTouchCheckKeepButton::ProcessCancel() {
    if (!(mFlags & 0x40) || !mTouched) return AnimButton::ProcessCancel();

    switch (mState) {
    case cState_OnStart:
    case cState_On:
    case cState_OffStart:
    case cState_DownStart:
    case cState_Down: return false;
    case cState_Off:
        StartCancel();
        ChangeState(cState_CancelStart);
        break;
    case cState_CancelStart: break;
    }

    return true;
}
}

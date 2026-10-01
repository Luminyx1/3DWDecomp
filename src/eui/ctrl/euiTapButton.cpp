#include <eui/euiTapButton.h>
#include <eui/euiLayoutEx.h>
#include <eui/euiScreen.h>
#include <eui/euiAnimatorSet.h>
#include <eui/euiAnimator.h>
#include <nn/ui2d/ui2d_Pane.h>
#include <eui/euiBoundingEx.h>
#include <eui/euiButtonGroup.h>
#include <eui/euiUtility.h>
#include <nn/ui2d/ui2d_ExtUserData.h>
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

// pHeap stores the animator set; pPane supplies hit geometry; pAnimator supplies the
// interaction animation, and pLayout provides ownership and the input mode.
void TapButton::Initialize(sead::Heap* pHeap, nn::ui2d::Pane* pPane, Animator* pAnimator, LayoutEx* pLayout) {
    _20 = pLayout;
    SetTouch(Screen::isTouchMode(pLayout->getScreen()));
    mHitPane = pPane;
    mStateAnimators = new (pHeap, 8) AnimatorSet;
    mStateAnimators->allocBuffer(6, pHeap);
    pAnimator->setSkipFirstFrame(true);
    pAnimator->setSoundLink(false);
    mStateAnimators->setAnimator(4, pAnimator);
    mName = pPane->mPanelName;
    mFlags |= 0x200;
}

// pPane supplies the interaction metadata; pLayout owns animations and pGroup receives the button.
void TapButton::CreateTapButton(nn::ui2d::Pane* pPane, LayoutEx* pLayout, ButtonGroup* pGroup) {
    if (DynamicCast<BoundingEx>(pPane) == nullptr) {
        return;
    }

    const auto* data = pPane->FindExtUserDataByName("TapButtonAnim");

    if (data == nullptr) {
        return;
    }

    Animator* animator = pLayout->tryCreateAnimatorAuto(static_cast<const char*>(data->GetData()), true);

    if (animator == nullptr) {
        return;
    }

    sead::Heap* heap = GetNwAllocatorHeap();
    auto* button = new (heap, 8) TapButton;
    button->Initialize(heap, pPane, animator, pLayout);
    pGroup->mButtons.push_back(*button);
}
}

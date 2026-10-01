#include <eui/euiAnimButton.h>
#include <eui/euiUtility.h>
#include <eui/euiAnimator.h>
#include <eui/euiAnimatorSet.h>
#include <eui/euiLayoutEx.h>
#include <eui/euiScreen.h>
#include <eui/euiButtonGroup.h>
#include <eui/euiBoxCursorNode.h>
#include <eui/euiRootPane.h>
#include <nn/ui2d/ui2d_ControlSrc.h>

namespace eui {

AnimButton::AnimButton() : mStateAnimators(nullptr), mDisableAnimator(nullptr),
                           mHitPane(nullptr), mCursorPane(nullptr) {
    mFlags |= 0x2000;
}

const char* AnimButton::getClassName() const { return "AnimButton"; }

// rSource names the panes, animations, and options; pLayout owns the button resources.
void AnimButton::Build(const nn::ui2d::ControlSrc& rSource, LayoutEx* pLayout) {
    _20 = pLayout;
    SetTouch(Screen::isTouchMode(pLayout->getScreen()));
    BuildStateAnim(rSource, pLayout);
    mStateAnimators->SetSkipFirstFrameAll(true);
    mStateAnimators->SetSoundLinkAll(false);
    const char* disableName = rSource.FindFunctionalAnimName("Disable");

    if (disableName == nullptr || !*disableName) {
        disableName = rSource.FindFunctionalAnimName("Invalid");
    }

    if (disableName != nullptr && *disableName) {
        mDisableAnimator = pLayout->tryCreateAnimatorAutoWithWarning(disableName, true);

        if (mDisableAnimator != nullptr) {
            mDisableAnimator->setSoundLink(false);
        }
    }

    const char* hitName = rSource.FindFunctionalPaneName("Hit");
    mHitPane = pLayout->findPaneByName(hitName);
    const char* cursorName = rSource.FindFunctionalPaneName("Cursor");

    if (cursorName != nullptr && *cursorName && pLayout->getRootPane()->FindExtUserDataByName("BoxCursorOff") == nullptr) {
        mCursorPane = pLayout->findPaneByName(cursorName);
        sead::Heap* heap = GetNwAllocatorHeap();
        pLayout->getScreen()->createBoxCursorNode(heap)->initialize(this, pLayout->getScreen());
    }

    bool isRoot = DynamicCast<RootPane>(pLayout->getRootPane()) != nullptr;
    mName = isRoot ? pLayout->getLayoutName() : pLayout->getRootPane()->mPanelName;

    if (rSource.FindExtUserDataByName("RepeatOn") != nullptr) {
        mFlags |= 0x80;
    }

    if (rSource.FindExtUserDataByName("NoTrigTouchOn") != nullptr) {
        mFlags |= 0x100;
    }

    if (rSource.FindExtUserDataByName("DownWithTouchOn") != nullptr) {
        mFlags |= 0x200;
    }
}

// rSource supplies normal and touch animation names; pLayout creates their animators.
void AnimButton::BuildStateAnim(const nn::ui2d::ControlSrc& rSource, LayoutEx* pLayout) {
    const char* names[] = {
        rSource.FindFunctionalAnimName("On"), rSource.FindFunctionalAnimName("TouchOn"),
        rSource.FindFunctionalAnimName("Off"), rSource.FindFunctionalAnimName("TouchOff"),
        rSource.FindFunctionalAnimName("Decide"), rSource.FindFunctionalAnimName("TouchDecide")
    };

    mStateAnimators = pLayout->createAnimatorSet(names, 6, true);
}

// force temporarily switches a touch button to cursor input while pressing it.
bool AnimButton::DownOff(bool force) {
    if ((mFlags & 0x14) != 0x14) {
        return false;
    }

    Screen* screen = getLayout()->getScreen();

    if (screen != nullptr && screen->getButtonGroup()->IsExistExcludingDown()) {
        return false;
    }

    if (force && IsTouch()) {
        mFlags &= ~0x40;
        mFlags |= 0x800;
    }

    Down();

    if (!(mFlags & 0x1000) && (screen == nullptr || mCursorPane == nullptr || !screen->moveBoxCursorByButton(this))) {
        Off();
    }

    return true;
}

void AnimButton::InactivateByBoxCursor() {
    mFlags &= ~0x1000;
    Off();

    if (getLayout()->getScreen()->isTouchMode()) {
        mFlags |= 0x800;
    }
}

// state is the next button state, reported to the owning screen before it is stored.
void AnimButton::ChangeState(State state) {
    if (mState == state) {
        return;
    }

    if ((state == cState_Off || state == cState_OffStart) && (mFlags & 0x800)) {
        mFlags &= ~0x800;
        mFlags |= 0x40;
    }

    Screen* screen = getLayout()->getScreen();

    if (screen != nullptr) {
        screen->buttonStateChangeCallback(this, static_cast<State>(mState), state);
    }

    mState = state;
}

// rOther supplies source resources; pLayout owns the clone; pHeap holds animations and cursor nodes.
void AnimButton::CloneImpl_(const AnimButton& rOther, LayoutEx* pLayout, sead::Heap* pHeap) {
    _20 = pLayout;
    SetTouch(rOther.IsTouch());
    mStateAnimators = new (pHeap, 8) AnimatorSet(*rOther.mStateAnimators, pLayout, pHeap);
    mStateAnimators->SetSkipFirstFrameAll(true);
    mStateAnimators->SetSoundLinkAll(false);

    if (rOther.mDisableAnimator != nullptr) {
        mDisableAnimator = pLayout->tryCreateAnimatorAutoWithWarning(rOther.mDisableAnimator->getName(), true);

        if (mDisableAnimator != nullptr) {
            mDisableAnimator->setSoundLink(false);
        }
    }

    if (rOther.mHitPane != nullptr) {
        mHitPane = pLayout->findPaneByName(rOther.mHitPane->mPanelName);
    }

    if (rOther.mCursorPane != nullptr) {
        mCursorPane = pLayout->findPaneByName(rOther.mCursorPane->mPanelName);
        pLayout->getScreen()->createBoxCursorNode(pHeap)->initialize(this, pLayout->getScreen());
    }

    mName = pLayout->getRootName();
}

// touch selects touch input and clears the pending touch activation flag.
void AnimButton::SetTouch(bool touch) {
    if (touch) {
        mFlags |= 0x40;
    } else {
        mFlags &= ~0x40;
    }

    mFlags &= ~0x800;
}

void AnimButton::Down() {
    ButtonBase::Down();
    mFlags &= ~0x4000;
}

// rPosition is the pointer position in layout coordinates.
bool AnimButton::HitTest(const sead::Vector2f& rPosition) const {
    return (mHitPane != nullptr) && IsHitPane(rPosition, mHitPane);
}

// rPosition is the initial pointer position; the base animated button has no drag behavior.
void AnimButton::StartDrag(const sead::Vector2f& rPosition) {}
// pPosition is the current pointer position, unused by this base implementation.
void AnimButton::UpdateDrag(const sead::Vector2f* pPosition) {}
// pPosition is the final pointer position, unused by this base implementation.
void AnimButton::FinishDrag(const sead::Vector2f* pPosition) {}

void AnimButton::ActivateByBoxCursor() {
    mFlags &= ~0x1840;
    mFlags |= 0x1000;
    On();
}

bool AnimButton::ProcessCancel() { return true; }

// disabled selects forward (disable) or reverse (enable) playback.
void AnimButton::PlayDisableAnim(bool disabled) {
    if (mDisableAnimator != nullptr) {
        mDisableAnimator->PlayFromCurrent(Animator::cPlayType_OneTime, disabled ? 1.0f : -1.0f);
    }
}

// disabled chooses the final or initial frame without playing the transition.
void AnimButton::SetDisableAnimDirect(bool disabled) {
    if (mDisableAnimator != nullptr) {
        if (disabled) {
            mDisableAnimator->StopAtMax();
        } else {
            mDisableAnimator->StopAtMin();
        }
    }
}

bool AnimButton::IsPlayDisableAnim() const {
    if (mDisableAnimator == nullptr) {
        return false;
    }

    if (mDisableAnimator->getStep() > 0) {
        return true;
    }

    return mDisableAnimator->isFrameMax();
}

// index is the normal-input animation slot; its successor is the touch variant.
Animator* AnimButton::SelectStateAnim(int index) {
    if (IsTouch() && mStateAnimators->getAnimator(index + 1) != nullptr) {
        return mStateAnimators->select(index + 1);
    }

    return mStateAnimators->select(index);
}

void AnimButton::ForceOff() {
    ButtonBase::ForceOff();
    SelectStateAnim(0)->StopAtMin();
}

void AnimButton::ForceOn() {
    ButtonBase::ForceOn();
    SelectStateAnim(0)->StopAtMax();
}

void AnimButton::ForceDown() {
    ButtonBase::ForceDown();
    SelectStateAnim(4)->StopAtMax();
}

void AnimButton::StartOn() {
    Animator* pOff = mStateAnimators->getAnimator(2);
    Animator* pOn = SelectStateAnim(0);

    if (pOff != nullptr) {
        pOn->Play(Animator::cPlayType_OneTime, 1);
    } else {
        pOn->PlayFromCurrent(Animator::cPlayType_OneTime, 1);
    }
}

void AnimButton::StartOff() {
    if (mStateAnimators->getAnimator(2) != nullptr) {
        SelectStateAnim(2)->Play(Animator::cPlayType_OneTime, 1);
    } else {
        SelectStateAnim(0)->PlayFromCurrent(Animator::cPlayType_OneTime, -1);
    }
}

void AnimButton::StartDown() { SelectStateAnim(4)->Play(Animator::cPlayType_OneTime, 1); }

bool AnimButton::UpdateOn() {
    return mStateAnimators->getSelected()->isPlayEnd();
}

bool AnimButton::UpdateOff() {
    return mStateAnimators->getSelected()->isPlayEnd();
}

bool AnimButton::UpdateDown() {
    return mStateAnimators->getSelected()->isPlayEnd();
}

// NON_MATCHING: the state jump table matches, but transition blocks are ordered differently.
bool AnimButton::ProcessOn() {
    bool processed = true;

    switch (mState) {
    case cState_Off: case cState_OffStart: StartOn(); ChangeState(cState_OnStart); break;
    case cState_DownStart: case cState_CancelStart: processed = false; break;
    case cState_Down: if (IsTouch()) { StartOn(); ChangeState(cState_OnStart); } break;
    case cState_OnStart: case cState_On: break;
    }

    return processed;
}

bool AnimButton::ProcessOff() {
    switch (mState) {
    case cState_OnStart:
        StartOff(); ChangeState(cState_OffStart); break;
    case cState_On:
        StartOff(); ChangeState(cState_OffStart); break;
    case cState_DownStart:
        return IsTouch();
    }

    return true;
}

void AnimButton::FinishDown() {
    if (IsTouch()) {
        mStateAnimators->select((mStateAnimators->getAnimator(1) != nullptr) ? 1 : 0)->StopAtMin();
        ChangeState(cState_Down);
        ChangeState(cState_Off);
    } else {
        ChangeState(cState_Down);
        ChangeState(cState_On);
    }
}

// state is applied immediately, restoring touch input after leaving cursor mode.
void AnimButton::ForceChangeState(State state) {
    if (mState == state) {
        return;
    }

    if ((state == cState_Off || state == cState_OffStart) && (mFlags & 0x800)) {
        mFlags &= ~0x800;
        mFlags |= 0x40;
    }

    mState = state;
}
}  // namespace eui

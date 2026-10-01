#include <eui/euiCheckButton.h>
#include <eui/euiAnimator.h>
#include <eui/euiLayoutEx.h>
#include <nn/ui2d/ui2d_ControlSrc.h>
namespace eui {
// pFlags supplies the animator flags; mask identifies the bits to enable.
static inline void SetAnimatorFlags(u8* pFlags, u8 mask) { *pFlags |= mask; }

// pFlags supplies the animator flags; mask identifies the bits to disable.
static inline void ResetAnimatorFlags(u8* pFlags, u8 mask) { *pFlags &= ~mask; }


// rOther supplies button state; pLayout and pHeap receive the cloned animations.
CheckButton::CheckButton(const CheckButton& rOther, LayoutEx* pLayout, sead::Heap* pHeap)
    : mChecked(rOther.mChecked), mCheckAnimator(nullptr) {
    CloneImpl_(rOther, pLayout, pHeap);
    mCheckAnimator = pLayout->tryCreateAnimatorAutoWithWarning(rOther.mCheckAnimator->getName(), true);
    SetAnimatorFlags(&mCheckAnimator->mFlags, 0x10);
    ResetAnimatorFlags(&mCheckAnimator->mFlags, 0x20);
}

// rSource names the check animation; pLayout owns the resulting animator.
void CheckButton::Build(const nn::ui2d::ControlSrc& rSource, LayoutEx* pLayout) {
    AnimButton::Build(rSource, pLayout);
    const char* name = rSource.FindFunctionalAnimName("Check");
    mCheckAnimator = pLayout->tryCreateAnimatorAutoWithWarning(name, true);
    SetAnimatorFlags(&mCheckAnimator->mFlags, 0x10);
    ResetAnimatorFlags(&mCheckAnimator->mFlags, 0x20);
}

const char* CheckButton::getClassName() const { return "CheckButton"; }
// checked selects the check animation endpoint and updates the logical value.
void CheckButton::ForceSetChecked(bool checked) {
    mChecked = checked;

    if (mCheckAnimator != nullptr) {
        if (checked) {
            mCheckAnimator->StopAtMax();
        } else {
            mCheckAnimator->StopAtMin();
        }
    }
}

void CheckButton::StartDown() {
    AnimButton::StartDown();

    if (mCheckEnabled && !IsPlayDisableAnim()) {
        if (mCheckAnimator != nullptr) {
            mCheckAnimator->Play(Animator::cPlayType_OneTime, mChecked ? -1.0f : 1.0f);
        }

        mChecked = !mChecked;
    }
}

bool CheckButton::UpdateDown() {
    bool finished = AnimButton::UpdateDown();

    if (mCheckEnabled && mCheckAnimator != nullptr && !IsPlayDisableAnim()) {
        if (mChecked) {
            finished = finished && mCheckAnimator->isFrameMax();
        } else {
            finished = finished && mCheckAnimator->getFrame() == 0;
        }
    }

    return finished;
}
}

#include <eui/euiCheckKeepButton.h>
#include <eui/euiAnimator.h>
namespace eui {
const char* CheckKeepButton::getClassName() const { return "CheckKeepButton"; }
// rOther supplies the properties; pLayout owns the clone; pHeap holds its animations.
CheckKeepButton::CheckKeepButton(const CheckKeepButton& rOther, LayoutEx* pLayout, sead::Heap* pHeap)
    : CheckButton(rOther, pLayout, pHeap) {}
void CheckKeepButton::Uncheck() {
    if (mCheckEnabled && mChecked && !IsPlayDisableAnim()) {
        if (mCheckAnimator != nullptr) mCheckAnimator->Play(Animator::cPlayType_OneTime, -1);
        mChecked = false;
    }
}

void CheckKeepButton::StartDown() {
    AnimButton::StartDown();

    if (mCheckEnabled && !mChecked && !IsPlayDisableAnim()) {
        if (mCheckAnimator != nullptr) mCheckAnimator->Play(Animator::cPlayType_OneTime, 1);
        mChecked = true;
    }
}
}

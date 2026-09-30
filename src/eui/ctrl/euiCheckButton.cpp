#include <eui/euiCheckButton.h>
#include <eui/euiAnimator.h>
namespace eui {
const char* CheckButton::getClassName() const { return "CheckButton"; }
// checked selects the check animation endpoint and updates the logical value.
void CheckButton::ForceSetChecked(bool checked) {
    mChecked = checked;
    if (mCheckAnimator) {
        if (checked) mCheckAnimator->StopAtMax();
        else mCheckAnimator->StopAtMin();
    }
}
void CheckButton::StartDown() {
    AnimButton::StartDown();
    if (mCheckEnabled && !IsPlayDisableAnim()) {
        if (mCheckAnimator)
            mCheckAnimator->Play(Animator::cPlayType_OneTime, mChecked ? -1.0f : 1.0f);
        mChecked = !mChecked;
    }
}
bool CheckButton::UpdateDown() {
    bool finished = AnimButton::UpdateDown();
    if (mCheckEnabled && mCheckAnimator && !IsPlayDisableAnim()) {
        if (mChecked)
            finished = finished && mCheckAnimator->mFrame == mCheckAnimator->GetFrameSize();
        else
            finished = finished && mCheckAnimator->mFrame == 0;
    }
    return finished;
}
}

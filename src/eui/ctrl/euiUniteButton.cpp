#include <eui/euiUniteButton.h>
#include <eui/euiAnimator.h>
namespace eui {
UniteButton::UniteButton() : mCheckAnimator(nullptr), mDragAnimator(nullptr),
    mDragStart(0, 0), mPaneStart(0, 0), mButtonType(ButtonType::cButtonType_4), mChecked(false),
    mDragHorizontal(true), mDragVertical(true) {}
const char* UniteButton::getClassName() const { return "UniteButton"; }
// checked selects the check animation's last frame or first frame.
void UniteButton::ForceSetChecked(bool checked) {
    mChecked = checked;
    if (mCheckAnimator) {
        if (checked) mCheckAnimator->StopAtMax();
        else mCheckAnimator->StopAtMin();
    }
}
}

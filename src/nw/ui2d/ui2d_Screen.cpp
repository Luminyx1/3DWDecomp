#include <nn/ui2d/ui2d_Screen.h>
#include <nn/ui2d/ui2d_AnimatorEx.h>
namespace nn::ui2d {
// animator is added to this screen's active update list.
void Screen::SetAnimatorActive(AnimatorEx* animator) { mActiveAnimators.LinkPrev(&animator->mActiveLink); }
// animator is detached from the active update list without destruction.
void Screen::EraseAnimatorFromActiveList(AnimatorEx* animator) {
    if (&mActiveAnimators != &animator->mActiveLink) animator->mActiveLink.Unlink();
}

ControlCreator* Screen::GetControlCreator() const { return mControlCreator; }
}

#include <nn/ui2d/ui2d_CheckButton.h>
#include <nn/ui2d/ui2d_Animator.h>
#include <nn/ui2d/ui2d_ControlSrc.h>
#include <nn/ui2d/ui2d_Layout.h>
namespace nn::ui2d {
// device creates animation resources, layout owns their targets, and source
// maps the control's functional animation names to layout resources.
void CheckButton::Build(nn::gfx::Device* device, Layout* layout, const ControlSrc& source) {
    AnimButton::Build(device, layout, source);
    mCheckAnimator = layout->CreateGroupAnimatorAuto(device, source.FindFunctionalAnimName("Check"), true);
    mCheckAnimator->StopAtStartFrame();
}

// checked selects the check animation's final or initial frame immediately.
void CheckButton::ForceSetChecked(bool checked) {
    mChecked = checked;

    if (mCheckAnimator) {
        if (checked) mCheckAnimator->StopAtEndFrame();
        else mCheckAnimator->StopAtStartFrame();
    }
}

void CheckButton::FinishDown() {
    ChangeState(cState_Down);
    ChangeState(cState_On);
}

void CheckButton::StartDown() {
    AnimButton::StartDown();

    if (mCheckAnimator) mCheckAnimator->Play(Animator::PlayType_Once, mChecked ? -1.0f : 1.0f);
    mChecked = !mChecked;
}

bool CheckButton::UpdateDown() {
    bool finished = AnimButton::UpdateDown();

    if (mCheckAnimator) {
        if (mChecked)
            finished = mDownAnimator->mFrame == float(mDownAnimator->GetFrameSize()) &&
                       mCheckAnimator->mFrame == float(mCheckAnimator->GetFrameSize());
        else
            finished = mDownAnimator->mFrame == float(mDownAnimator->GetFrameSize()) &&
                       mCheckAnimator->mFrame == 0.0f;
    }

    return finished;
}
}

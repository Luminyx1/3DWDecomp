#pragma once
#include <nn/ui2d/ui2d_AnimButton.h>
namespace nn::ui2d {
class CheckButton : public AnimButton {
public:
    CheckButton() : mChecked(false), mCheckAnimator(nullptr) {}
    NN_RUNTIME_TYPEINFO(AnimButton);
    void Build(nn::gfx::Device* device, Layout* layout, const ControlSrc& source);
    void ForceSetChecked(bool checked);
    void FinishDown() override;
    void StartDown() override;
    bool UpdateDown() override;
    bool mChecked;
    Animator* mCheckAnimator;
};
static_assert(sizeof(CheckButton) == 0xa0, "CheckButton size");
}

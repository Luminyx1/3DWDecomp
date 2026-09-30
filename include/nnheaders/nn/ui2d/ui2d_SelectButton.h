#pragma once
#include <nn/ui2d/ui2d_AnimButton.h>
namespace nn::ui2d {
class SelectButton : public AnimButton {
public:
    NN_RUNTIME_TYPEINFO(AnimButton);
    void Build(nn::gfx::Device* device, Layout* layout, const ControlSrc& source);
    bool IsHit(const nn::util::Float2& position) const override;
    bool ProcessOff() override;
    bool ProcessCancel() override;
    void FinishCancel() override;
};
}

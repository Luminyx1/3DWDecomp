#pragma once
#include <nn/ui2d/ui2d_DragButton.h>
namespace nn::ui2d {
class TouchDragButton : public DragButton {
public:
    TouchDragButton();
    NN_RUNTIME_TYPEINFO(DragButton);
    void Build(nn::gfx::Device* device, Layout* layout, const ControlSrc& source);
    bool ProcessOn() override;
    bool ProcessCancel() override;
    void FinishCancel() override;
};
}

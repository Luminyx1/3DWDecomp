#pragma once
#include <nn/ui2d/ui2d_AnimButton.h>
namespace nn::ui2d {
class DragButton : public AnimButton {
public:
    DragButton();
    NN_RUNTIME_TYPEINFO(AnimButton);
    void Build(nn::gfx::Device* device, Layout* layout, const ControlSrc& source);
    void InitializeDragPosition(const nn::util::Float2& position) override;
    void UpdateDragPosition(const nn::util::Float2* position) override;
    bool ProcessOff() override;
    bool ProcessCancel() override;
    void FinishCancel() override;
    Pane* mDragPane;
    nn::util::Float2 mDragStart;
    nn::util::Float2 mPaneStart;
    bool mDragX;
    bool mDragY;
};
static_assert(sizeof(DragButton) == 0xb0, "DragButton size");
}

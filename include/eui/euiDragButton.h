#pragma once
#include <eui/euiSelectButton.h>
namespace eui {
class DragButton : public SelectButton {
public:
    DragButton();
    DragButton(const DragButton& rOther, LayoutEx* pLayout, sead::Heap* pHeap);
    ~DragButton() override = default;
    const char* getClassName() const override;
    NN_RUNTIME_TYPEINFO(SelectButton);
    void Build(const nn::ui2d::ControlSrc& rSource, LayoutEx* pLayout) override;
    void BuildStateAnim(const nn::ui2d::ControlSrc& rSource, LayoutEx* pLayout) override;
    void StartDrag(const sead::Vector2f& rPosition) override;
    void UpdateDrag(const sead::Vector2f* pPosition) override;
    void FinishDrag(const sead::Vector2f* pPosition) override;
    void StartCancel() override;
    void FinishCancel() override;
    nn::ui2d::Pane* mDragPane;
    sead::Vector2f mDragStart;
    sead::Vector2f mPaneStart;
    bool mDragX;
    bool mDragY;
};

static_assert(sizeof(DragButton) == 0x88, "DragButton size");
}

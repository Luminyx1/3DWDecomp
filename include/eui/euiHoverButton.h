#pragma once
#include <eui/euiAnimButton.h>

namespace eui {
class ButtonGroup;
class HoverButton : public AnimButton {
public:
    ~HoverButton() override = default;
    const char* getClassName() const override;
    NN_RUNTIME_TYPEINFO(AnimButton);
    void Down() override;
    void Initialize(sead::Heap* pHeap, nn::ui2d::Pane* pPane, Animator* pAnimator, LayoutEx* pLayout);
    static HoverButton* CreateHoverButton(nn::ui2d::Pane* pPane, LayoutEx* pLayout, ButtonGroup* pGroup);
};
}

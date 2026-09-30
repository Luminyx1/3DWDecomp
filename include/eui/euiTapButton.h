#pragma once
#include <eui/euiAnimButton.h>

namespace eui {
class ButtonGroup;
class TapButton : public AnimButton {
public:
    ~TapButton() override = default;
    const char* getClassName() const override;
    NN_RUNTIME_TYPEINFO(AnimButton);
    void On() override;
    void Off() override;
    bool ProcessDown() override;
    void FinishDown() override;
    void ForceOff() override;
    void Initialize(sead::Heap* pHeap, nn::ui2d::Pane* pPane, Animator* pAnimator, LayoutEx* pLayout);
    static TapButton* CreateTapButton(nn::ui2d::Pane* pPane, LayoutEx* pLayout, ButtonGroup* pGroup);
};
}

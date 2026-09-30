#pragma once
#include <eui/euiCheckKeepButton.h>
namespace eui {
class TwoTouchCheckKeepButton : public CheckKeepButton {
public:
    TwoTouchCheckKeepButton();
    TwoTouchCheckKeepButton(const TwoTouchCheckKeepButton& rOther, LayoutEx* pLayout, sead::Heap* pHeap);
    ~TwoTouchCheckKeepButton() override = default;
    const char* getClassName() const override;
    NN_RUNTIME_TYPEINFO(CheckKeepButton);
    bool IsTouchOnce() const;
    void ForceSetTouchOnce(bool touched);
    bool HitTest(const sead::Vector2f& rPosition) const override;
    void ActivateByBoxCursor() override;
    void InactivateByBoxCursor() override;
    void BuildStateAnim(const nn::ui2d::ControlSrc& rSource, LayoutEx* pLayout) override;
    bool ProcessCancel() override;
    void StartDown() override;
    bool UpdateDown() override;
    void FinishDown() override;
    void StartCancel() override;
    bool UpdateCancel() override;
    void FinishCancel() override;
    AnimatorSet* mFirstTouchAnimators;
    AnimatorSet* mSecondTouchAnimators;
    u8 mTouched;
};

static_assert(sizeof(TwoTouchCheckKeepButton) == 0x90, "TwoTouchCheckKeepButton size");
}

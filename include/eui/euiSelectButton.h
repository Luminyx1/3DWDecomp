#pragma once
#include <eui/euiAnimButton.h>

namespace eui {
class SelectButton : public AnimButton {
public:
    SelectButton() = default;
    SelectButton(const SelectButton& rOther, LayoutEx* pLayout, sead::Heap* pHeap);
    ~SelectButton() override = default;
    const char* getClassName() const override;
    NN_RUNTIME_TYPEINFO(AnimButton);
    void BuildStateAnim(const nn::ui2d::ControlSrc& rSource, LayoutEx* pLayout) override;
    bool HitTest(const sead::Vector2f& rPosition) const override;
    bool ProcessOn() override;
    bool ProcessOff() override;
    void StartCancel() override;
    bool UpdateCancel() override;
    bool ProcessCancel() override;
    void FinishDown() override;
    void FinishCancel() override;
};
}

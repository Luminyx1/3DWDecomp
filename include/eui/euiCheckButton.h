#pragma once
#include <eui/euiAnimButton.h>
namespace eui {
class CheckButton : public AnimButton {
public:
    CheckButton() : mChecked(false), mCheckEnabled(true), mCheckAnimator(nullptr) {}
    CheckButton(const CheckButton& rOther, LayoutEx* pLayout, sead::Heap* pHeap);
    ~CheckButton() override = default;
    const char* getClassName() const override;
    NN_RUNTIME_TYPEINFO(AnimButton);
    void Build(const nn::ui2d::ControlSrc& rSource, LayoutEx* pLayout) override;
    void ForceSetChecked(bool checked);
    void StartDown() override;
    bool UpdateDown() override;
    bool mChecked;
    bool mCheckEnabled;
    Animator* mCheckAnimator;
};

static_assert(sizeof(CheckButton) == 0x78, "CheckButton size");
}

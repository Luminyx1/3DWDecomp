#pragma once
#include <eui/euiCheckButton.h>
namespace eui {
class CheckKeepButton : public CheckButton {
public:
    CheckKeepButton() = default;
    CheckKeepButton(const CheckKeepButton& rOther, LayoutEx* pLayout, sead::Heap* pHeap);
    ~CheckKeepButton() override = default;
    const char* getClassName() const override;
    NN_RUNTIME_TYPEINFO(CheckButton);
    void StartDown() override;
    virtual void Uncheck();
};
}

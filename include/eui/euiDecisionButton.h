#pragma once

#include <eui/euiAnimButton.h>

namespace eui {

class DecisionButton : public AnimButton {
public:
    DecisionButton() { mFlags |= 0x20; }
    DecisionButton(const DecisionButton& rOther, LayoutEx* pLayout, sead::Heap* pHeap);
    ~DecisionButton() override = default;
    const char* getClassName() const override;
    NN_RUNTIME_TYPEINFO(AnimButton);
    bool ProcessOn() override;
    bool ProcessOff() override;
    void FinishDown() override;
};

}  // namespace eui

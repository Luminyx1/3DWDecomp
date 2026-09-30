#pragma once
#include <nn/ui2d/ui2d_AnimButton.h>
namespace nn::ui2d {
class DecisionButton : public AnimButton {
public:
    DecisionButton() { mFlags |= 0x20; }
    NN_RUNTIME_TYPEINFO(AnimButton);
    bool ProcessOn() override;
    bool ProcessOff() override;
};
}

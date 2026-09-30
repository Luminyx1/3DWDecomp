#pragma once
#include <nn/ui2d/ui2d_AnimButton.h>
namespace nn::ui2d {
class TouchOffButton : public AnimButton {
public:
    NN_RUNTIME_TYPEINFO(AnimButton);
    bool ProcessOn() override;
    bool ProcessOff() override;
    void FinishDown() override;
};
}

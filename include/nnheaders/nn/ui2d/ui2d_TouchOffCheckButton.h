#pragma once
#include <nn/ui2d/ui2d_CheckButton.h>
namespace nn::ui2d {
class TouchOffCheckButton : public CheckButton {
public:
    NN_RUNTIME_TYPEINFO(CheckButton);
    bool ProcessOn() override;
    bool ProcessOff() override;
    void FinishDown() override;
};
}

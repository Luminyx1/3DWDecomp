#include <nn/ui2d/ui2d_NormalButton.h>
namespace nn::ui2d {
NormalButton::NormalButton() = default;

// Return to the selected state after completing a press.
void NormalButton::FinishDown() {
    ChangeState(cState_Down);
    ChangeState(cState_On);
}
NormalButtonEx::NormalButtonEx() : mLayout(nullptr) {}

// device creates the cloned animators, source supplies their bindings, and
// layout supplies the destination panes and animation resources.
NormalButtonEx::NormalButtonEx(nn::gfx::Device* device, const NormalButtonEx& source, Layout* layout)
    : mLayout(nullptr) {
    CloneImpl_(device, source, layout);
}
void NormalButtonEx::FinishDown() {
    ChangeState(cState_Down);
    ChangeState(cState_On);
}
}

#include <nn/ui2d/ui2d_DefaultControlCreator.h>
#include <nn/ui2d/ui2d_ButtonGroup.h>
#include <nn/ui2d/ui2d_NormalButton.h>
#include <nn/ui2d/ui2d_DecisionButton.h>
#include <nn/ui2d/ui2d_SelectButton.h>
#include <nn/ui2d/ui2d_CheckButton.h>
#include <nn/ui2d/ui2d_TouchOffButton.h>
#include <nn/ui2d/ui2d_TouchOffCheckButton.h>
#include <nn/ui2d/ui2d_DragButton.h>
#include <nn/ui2d/ui2d_TouchDragButton.h>
#include <nn/ui2d/ui2d_ControlSrc.h>
#include <nn/ui2d/ui2d_LayoutEx.h>
#include <cstring>
#include <new>
namespace nn::ui2d {
namespace {
template<class T> T* AllocateControl() {
    void* memory = Layout::AllocateMemory(sizeof(T));
    return memory ? new (memory) T : nullptr;
}
// layout is checked against the runtime type hierarchy before using its extended interface.
LayoutEx* AsLayoutEx(Layout* layout) {
    const auto* wanted = LayoutEx::GetRuntimeTypeInfoStatic();
    if (!layout) return nullptr;
    auto* type = layout->GetRuntimeTypeInfo();
    while (type && type != wanted) type = type->m_ParentTypeInfo;
    return type ? static_cast<LayoutEx*>(layout) : nullptr;
}
}
// buttons receives the button controls created from layout resources.
DefaultControlCreator::DefaultControlCreator(ButtonGroup* buttons) : mButtons(buttons) {}
// device owns resources, layout owns panes, and source describes the requested control.
void DefaultControlCreator::CreateControl(nn::gfx::Device* device, Layout* layout, const ControlSrc& source) {
    if (!mButtons) return;
    AnimButton* button;
    if (std::strcmp("NormalButton", source.mName) == 0) {
        if (AsLayoutEx(layout)) {
            button = AllocateControl<NormalButtonEx>(); button->BuildEx(device, layout, source);
        } else {
            button = AllocateControl<NormalButton>(); button->Build(device, layout, source);
        }
    } else if (std::strcmp("DecisionButton", source.mName) == 0) {
        button = AllocateControl<DecisionButton>(); button->Build(device, layout, source);
    } else if (std::strcmp("SelectButton", source.mName) == 0) {
        auto* value = AllocateControl<SelectButton>(); value->Build(device, layout, source); button = value;
    } else if (std::strcmp("CheckButton", source.mName) == 0) {
        auto* value = AllocateControl<CheckButton>(); value->Build(device, layout, source); button = value;
    } else if (std::strcmp("TouchOffButton", source.mName) == 0) {
        button = AllocateControl<TouchOffButton>(); button->Build(device, layout, source);
    } else if (std::strcmp("TouchOffCheckButton", source.mName) == 0) {
        auto* value = AllocateControl<TouchOffCheckButton>(); value->Build(device, layout, source); button = value;
    } else if (std::strcmp("DragButton", source.mName) == 0) {
        auto* value = AllocateControl<DragButton>(); value->Build(device, layout, source); button = value;
    } else if (std::strcmp("TouchDragButton", source.mName) == 0) {
        auto* value = AllocateControl<TouchDragButton>(); value->Build(device, layout, source); button = value;
    } else return;
    if (button) mButtons->mButtons.push_back(*button);
}
DefaultControlCreatorEx::DefaultControlCreatorEx() : DefaultControlCreator(nullptr), mControls(nullptr) {}
// buttons and controls receive the two categories of created controls.
DefaultControlCreatorEx::DefaultControlCreatorEx(ButtonGroup* buttons, ControlList* controls) : DefaultControlCreator(buttons), mControls(controls) {}
// device and layout own resources; source selects a gauge or a standard button.
void DefaultControlCreatorEx::CreateControl(nn::gfx::Device* device, Layout* layout, const ControlSrc& source) {
    if (std::strcmp("TraceGaugeControl", source.mName) != 0) {
        DefaultControlCreator::CreateControl(device, layout, source); return;
    }
    LayoutEx* extended = AsLayoutEx(layout);
    auto* control = AllocateControl<TraceGaugeControl>();
    control->Initialize(device, source, extended);
    if (control) mControls->push_back(*control);
}
}

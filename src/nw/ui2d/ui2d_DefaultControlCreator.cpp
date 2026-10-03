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
/**
 * @brief Allocate storage and construct a layout control with its default settings.
 * @tparam T Control type whose size determines the allocation.
 * @return Constructed control, or nullptr when layout allocation fails.
 */
template <class T> inline T* AllocateControl() {
    void* memory = Layout::AllocateMemory(sizeof(T));
    return memory != nullptr ? new (memory) T : nullptr;
}

/**
 * @brief Obtain the extended layout interface after checking its runtime type hierarchy.
 * @param layout Layout to inspect; may be nullptr.
 * @return Extended layout, or nullptr for a null or incompatible layout.
 */
inline LayoutEx* AsLayoutEx(Layout* layout) {
    const auto* wanted = LayoutEx::GetRuntimeTypeInfoStatic();

    if (layout == nullptr) {
        return nullptr;
    }
    auto* type = layout->GetRuntimeTypeInfo();

    while (type != nullptr) {
        if (type == wanted) {
            return static_cast<LayoutEx*>(layout);
        }
        type = type->m_ParentTypeInfo;
    }
    return nullptr;
}
} // namespace

/**
 * @brief Create a factory that registers buttons with the supplied group.
 * @param buttons Destination group; nullptr disables standard button creation.
 */
DefaultControlCreator::DefaultControlCreator(ButtonGroup* buttons) : mButtons(buttons) {}
/**
 * @brief Construct and register a recognized button described by a control resource.
 * @param device Graphics device used when building the button's animations.
 * @param layout Layout containing the panes and animations referenced by the resource.
 * @param source Control resource whose name selects the button type; unknown names are ignored.
 */
void DefaultControlCreator::CreateControl(nn::gfx::Device* device, Layout* layout, const ControlSrc& source) {
    if (mButtons == nullptr) {
        return;
    }
    AnimButton* button;

    if (std::strcmp("NormalButton", source.mName) == 0) {
        if (AsLayoutEx(layout) != nullptr) {
            button = AllocateControl<NormalButtonEx>();
            button->BuildEx(device, layout, source);
        } else {
            button = AllocateControl<NormalButton>();
            button->Build(device, layout, source);
        }
    } else if (std::strcmp("DecisionButton", source.mName) == 0) {
        button = AllocateControl<DecisionButton>();
        button->Build(device, layout, source);
    } else if (std::strcmp("SelectButton", source.mName) == 0) {
        auto* value = AllocateControl<SelectButton>();
        value->Build(device, layout, source);
        button = value;
    } else if (std::strcmp("CheckButton", source.mName) == 0) {
        auto* value = AllocateControl<CheckButton>();
        value->Build(device, layout, source);
        button = value;
    } else if (std::strcmp("TouchOffButton", source.mName) == 0) {
        button = AllocateControl<TouchOffButton>();
        button->Build(device, layout, source);
    } else if (std::strcmp("TouchOffCheckButton", source.mName) == 0) {
        auto* value = AllocateControl<TouchOffCheckButton>();
        value->Build(device, layout, source);
        button = value;
    } else if (std::strcmp("DragButton", source.mName) == 0) {
        auto* value = AllocateControl<DragButton>();
        value->Build(device, layout, source);
        button = value;
    } else if (std::strcmp("TouchDragButton", source.mName) == 0) {
        auto* value = AllocateControl<TouchDragButton>();
        value->Build(device, layout, source);
        button = value;
    } else {
        return;
    }

    if (button != nullptr) {
        mButtons->mButtons.push_back(*button);
    }
}

/** @brief Construct a factory with no button group or control list attached. */
DefaultControlCreatorEx::DefaultControlCreatorEx() : DefaultControlCreator(nullptr), mControls(nullptr) {}
/**
 * @brief Create a factory for standard buttons and extended display controls.
 * @param buttons Destination button group; nullptr disables standard button creation.
 * @param controls Destination control list; must be valid when creating trace gauges.
 */
DefaultControlCreatorEx::DefaultControlCreatorEx(ButtonGroup* buttons, ControlList* controls)
    : DefaultControlCreator(buttons), mControls(controls) {}
/**
 * @brief Construct a trace gauge or delegate other names to the standard button factory.
 * @param device Graphics device used to create animation resources.
 * @param layout Owning layout; must support LayoutEx when the resource selects a trace gauge.
 * @param source Control resource selecting the type and its pane and animation bindings.
 */
void DefaultControlCreatorEx::CreateControl(nn::gfx::Device* device, Layout* layout,
                                            const ControlSrc& source) {
    if (std::strcmp("TraceGaugeControl", source.mName) != 0) {
        DefaultControlCreator::CreateControl(device, layout, source);
        return;
    }

    LayoutEx* extended = AsLayoutEx(layout);
    auto* control = AllocateControl<TraceGaugeControl>();
    control->Initialize(device, source, extended);

    if (control != nullptr) {
        mControls->push_back(*control);
    }
}
} // namespace nn::ui2d

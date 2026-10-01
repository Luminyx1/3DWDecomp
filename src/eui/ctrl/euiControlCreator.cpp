#include <eui/euiControlCreator.h>
#include <eui/euiButtonGroup.h>
#include <eui/euiAnimButton.h>
#include <eui/euiLayoutEx.h>
#include <eui/euiNormalButton.h>
#include <eui/euiDecisionButton.h>
#include <eui/euiSelectButton.h>
#include <eui/euiCheckButton.h>
#include <eui/euiDragButton.h>
#include <eui/euiCheckKeepButton.h>
#include <eui/euiTwoTouchCheckKeepButton.h>
#include <eui/euiUniteButton.h>
#include <eui/euiBoxCursorControl.h>
#include <eui/euiTraceGaugeControl.h>
#include <eui/euiDragScrollButton.h>
#include <nn/ui2d/ui2d_ControlSrc.h>
#include <cstring>
#include <new>
namespace eui {
template <class T>
static T* AllocateControl() {
    void* memory = nn::ui2d::Layout::AllocateMemory(sizeof(T));
    return memory ? new (memory) T : nullptr;
}

// rSource selects the control type and its resources; pLayout owns the resulting control.
ControlBase* ControlCreator::CreateControlImpl_(const nn::ui2d::ControlSrc& rSource, nn::ui2d::Layout* pLayout) {
    const char* name = rSource.mName;
    auto* layout = static_cast<LayoutEx*>(pLayout);
    AnimButton* button = nullptr;
    ControlBase* control = nullptr;

    if (std::strcmp("NormalButton", name) == 0) button = AllocateControl<NormalButton>();
    else if (std::strcmp("DecisionButton", name) == 0) button = AllocateControl<DecisionButton>();
    else if (std::strcmp("SelectButton", name) == 0) button = AllocateControl<SelectButton>();
    else if (std::strcmp("CheckButton", name) == 0) button = AllocateControl<CheckButton>();
    else if (std::strcmp("DragButton", name) == 0) button = AllocateControl<DragButton>();
    else if (std::strcmp("CheckKeepButton", name) == 0) button = AllocateControl<CheckKeepButton>();
    else if (std::strcmp("TwoTouchCheckKeepButton", name) == 0) button = AllocateControl<TwoTouchCheckKeepButton>();
    else if (std::strcmp("UniteButton", name) == 0) button = AllocateControl<UniteButton>();
    else if (std::strcmp("BoxCursor", name) == 0) {
        auto* cursor = AllocateControl<BoxCursorControl>();
        cursor->initialize(rSource, layout);
        control = cursor;
        goto registerControl;
    } else if (std::strcmp("TraceGauge", name) == 0) {
        auto* gauge = AllocateControl<TraceGaugeControl>();
        gauge->initialize(rSource, layout);
        control = gauge;
        goto registerControl;
    } else if (std::strcmp("DragScrollButton", name) == 0) button = AllocateControl<DragScrollButton>();
    else return nullptr;
    button->Build(rSource, layout);

    if (button != nullptr) InsertButtonToButtonGroup_(button);
    return button;
registerControl:
    if (control != nullptr) mControls->push_back(*control);
    return control;
}

// NON_MATCHING: backward traversal and insertion branches still differ.
// pButton is inserted before the trailing buttons belonging to its child layouts.
void ControlCreator::InsertButtonToButtonGroup_(AnimButton* pButton) {
    auto& buttons = mButtons->mButtons;
    ControlBase* insertBefore = nullptr;

    if (!buttons.empty()) {
        auto* current = &buttons.back();
        auto* layout = pButton->_20;

        if (static_cast<LayoutEx*>(current->_20)->mParentLayout == layout) {
            do {
                if (current == &*buttons.begin()) {
                    buttons.push_front(*pButton);
                    return;
                }

                insertBefore = current;
                current = &nn::util::IntrusiveListMemberNodeTraits<ControlBase, &ControlBase::m_Link>::GetItem(*current->m_Link.GetPrev());
            } while (static_cast<LayoutEx*>(current->_20)->mParentLayout == layout);
        }
    }

    if (insertBefore != nullptr) insertBefore->m_Link.LinkPrev(&pButton->m_Link);
    else buttons.push_back(*pButton);
}

// pButtons receives created buttons; pControls and pStaticControls receive the other controls.
ControlCreator::ControlCreator(ButtonGroup* pButtons, ControlList* pControls, ControlList* pStaticControls)
    : mButtons(pButtons), mControls(pControls), mStaticControls(pStaticControls) {}
// pDevice is unused here; rSource describes the control and pLayout owns its UI resources.
void ControlCreator::CreateControl(nn::gfx::Device* pDevice, nn::ui2d::Layout* pLayout,
                                   const nn::ui2d::ControlSrc& rSource) {
    CreateControlImpl_(rSource, pLayout);
}
}

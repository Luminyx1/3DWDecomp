#include <eui/euiControlCreator.h>
namespace eui {
// pButtons receives created buttons; pControls and pStaticControls receive the other controls.
ControlCreator::ControlCreator(ButtonGroup* pButtons, ControlList* pControls, ControlList* pStaticControls)
    : mButtons(pButtons), mControls(pControls), mStaticControls(pStaticControls) {}
// pDevice is unused here; rSource describes the control and pLayout owns its UI resources.
void ControlCreator::CreateControl(nn::gfx::Device* pDevice, nn::ui2d::Layout* pLayout,
                                   const nn::ui2d::ControlSrc& rSource) {
    CreateControlImpl_(rSource, pLayout);
}
}

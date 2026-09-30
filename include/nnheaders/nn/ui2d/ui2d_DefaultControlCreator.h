#pragma once
#include <nn/ui2d/ui2d_ControlCreator.h>
#include <nn/ui2d/ui2d_TraceGaugeControl.h>
namespace nn::ui2d {
class ButtonGroup;
using ControlList = nn::util::IntrusiveList<ControlBase, nn::util::IntrusiveListMemberNodeTraits<ControlBase, &ControlBase::m_Link>>;
class DefaultControlCreator : public ControlCreator {
public:
    explicit DefaultControlCreator(ButtonGroup* buttons);
    void CreateControl(nn::gfx::Device* device, Layout* layout, const ControlSrc& source) override;
    ButtonGroup* mButtons;
};
class DefaultControlCreatorEx : public DefaultControlCreator {
public:
    DefaultControlCreatorEx();
    DefaultControlCreatorEx(ButtonGroup* buttons, ControlList* controls);
    void CreateControl(nn::gfx::Device* device, Layout* layout, const ControlSrc& source) override;
    ControlList* mControls;
};
}

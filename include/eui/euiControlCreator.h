#pragma once
#include <eui/euiControlBase.h>
#include <nn/ui2d/ui2d_ControlCreator.h>
namespace eui {
class ButtonGroup;
class AnimButton;
using ControlList = nn::util::IntrusiveList<ControlBase,
    nn::util::IntrusiveListMemberNodeTraits<ControlBase, &ControlBase::m_Link>>;
class ControlCreator : public nn::ui2d::ControlCreator {
public:
    ControlCreator(ButtonGroup* pButtons, ControlList* pControls, ControlList* pStaticControls);
    ~ControlCreator() override = default;
    void CreateControl(nn::gfx::Device* pDevice, nn::ui2d::Layout* pLayout,
                       const nn::ui2d::ControlSrc& rSource) override;
    virtual ControlBase* CreateControlImpl_(const nn::ui2d::ControlSrc& rSource, nn::ui2d::Layout* pLayout);
    virtual void InsertButtonToButtonGroup_(AnimButton* pButton);
    ButtonGroup* mButtons;
    ControlList* mControls;
    ControlList* mStaticControls;
};
static_assert(sizeof(ControlCreator) == 0x20, "ControlCreator size");
}

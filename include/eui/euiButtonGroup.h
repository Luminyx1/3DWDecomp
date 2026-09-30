#pragma once
#include <eui/euiControlCreator.h>
#include <math/seadVector.h>
namespace eui {
class ButtonGroup {
public:
    ButtonGroup();
    virtual ~ButtonGroup();
    virtual void Update(const sead::Vector2f* pPosition, bool triggered, bool held, bool released);
    void SetTouchDevice(bool touch);
    AnimButton* FindDownButton();
    void ForceOffAll();
    void ForceOnAll();
    void ForceDownAll();
    void CancelAll();
    void SetAllowNoTrigTouchAll(bool allow);
    void SetDownWithTouchOnAll(bool enabled);
    ControlList mButtons;
    nn::util::IntrusiveListNode mUpdateList;
    AnimButton* mHitButton;
    AnimButton* mDownButton;
    AnimButton* mDragButton;
    u32 mFlags;
};
static_assert(sizeof(ButtonGroup) == 0x48, "ButtonGroup size");
}

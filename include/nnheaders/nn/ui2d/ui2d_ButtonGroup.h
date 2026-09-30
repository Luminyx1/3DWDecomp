#pragma once
#include <nn/ui2d/ui2d_AnimButton.h>
namespace nn::ui2d {
class ButtonGroup {
public:
    using ButtonList = nn::util::IntrusiveList<AnimButton, nn::util::IntrusiveListMemberNodeTraits<AnimButton, &AnimButton::mLink>>;
    ButtonGroup();
    virtual ~ButtonGroup();
    virtual void Update(const nn::util::Float2* position, bool pressed, bool released);
    AnimButton* FindDownButton();
    AnimButton* FindButtonByName(const char* name);
    const AnimButton* FindButtonByName(const char* name) const;
    AnimButton* FindButtonByNameReverse(const char* name);
    const AnimButton* FindButtonByNameReverse(const char* name) const;
    AnimButton* FindButtonByTag(int tag);
    const AnimButton* FindButtonByTag(int tag) const;
    AnimButton* FindButtonByLayout(const Layout* layout);
    void ForceOffAll();
    void ForceOnAll();
    void ForceDownAll();
    void UpdateHitBoxAll();
    void SetStateChangeCallbackAll(AnimButton::StateChangeCallback callback, void* argument);
    void CancelAll();
    void FreeAll();
    bool IsExistExcludingDown() const {
        for (auto& button : mButtons)
            if ((button.mFlags & 0x20) && button.IsDowning()) return true;
        return false;
    }
    ButtonList mButtons;
    AnimButton* mSelected;
    AnimButton* mDragging;
    u32 mFlags;
};
static_assert(sizeof(ButtonGroup) == 0x30, "ButtonGroup size");
}

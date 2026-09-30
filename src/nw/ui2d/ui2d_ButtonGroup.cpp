#include <nn/ui2d/ui2d_ButtonGroup.h>
#include <nn/ui2d/ui2d_Layout.h>
namespace nn::ui2d {
namespace {
// name and candidate are compared through the first 64 bytes or terminating null.
inline bool SameName(const char* name, const char* candidate) {
    for (size_t i = 0; i < 64; ++i) {
        if (name[i] != candidate[i]) return false;
        if (!name[i]) return true;
    }

    return true;
}
}

ButtonGroup::ButtonGroup() : mSelected(nullptr), mDragging(nullptr), mFlags(7) {}
ButtonGroup::~ButtonGroup() = default;
// position is nullable pointer input; pressed and released are this frame's edges.
void ButtonGroup::Update(const nn::util::Float2* position, bool pressed, bool released) {
    if (mFlags & 1) UpdateHitBoxAll();
    bool allowPress = !(mFlags & 2) || !IsExistExcludingDown();
    bool allowHit = ((mFlags & 4) >> 2) & ((position != nullptr) & allowPress);
    AnimButton* selected = nullptr;
    if (mDragging) {
        if (allowHit && (mDragging->mFlags & 0x10) && mDragging->IsHit(*position)) selected = mDragging;
        mDragging->UpdateDragPosition(position);
        if (released) {
            mDragging->Cancel();
            if (selected != mDragging) mDragging->Off();
            mDragging = nullptr;
        }
    } else if (allowHit) {
        for (auto& button : mButtons) {
            if ((button.mFlags & 0x10) && button.IsHit(*position)) { selected = &button; break; }
        }
    }

    if (mSelected != selected) {
        if (mSelected) mSelected->Off();
        if (selected) selected->On();
        mSelected = selected;
    }

    if (selected) {
        bool down = allowPress && pressed;
        if (down) { selected->Down(); selected = mSelected; }
        u32 dragMode = (selected->mFlags >> 6) & 3;
        if ((dragMode == 2 || (dragMode == 1 && down)) && !mDragging && !released) {
            mDragging = mSelected;
            mDragging->InitializeDragPosition(*position);
        }
    }

    for (auto& button : mButtons) button.Update();
}

AnimButton* ButtonGroup::FindDownButton() {
    for (auto& button : mButtons) if (button.IsDowning()) return &button;
    return nullptr;
}

// name is the control name to locate in insertion order.
AnimButton* ButtonGroup::FindButtonByName(const char* name) {
    for (auto& button : mButtons) if (SameName(name, button.mName)) return &button;
    return nullptr;
}

// name is located from the newest button toward the oldest.
AnimButton* ButtonGroup::FindButtonByNameReverse(const char* name) {
    for (auto it = mButtons.rbegin(); it != mButtons.rend(); ++it)
        if (SameName(name, it->mName)) return &*it;
    return nullptr;
}

// tag is the integer identifier assigned to the desired button.
AnimButton* ButtonGroup::FindButtonByTag(int tag) {
    for (auto& button : mButtons) if (button.mTag == tag) return &button;
    return nullptr;
}

// name is the control name to locate in insertion order.
const AnimButton* ButtonGroup::FindButtonByName(const char* name) const {
    for (auto& button : mButtons) if (SameName(name, button.mName)) return &button;
    return nullptr;
}

// name is located from the newest button toward the oldest.
const AnimButton* ButtonGroup::FindButtonByNameReverse(const char* name) const {
    for (auto it = mButtons.rbegin(); it != mButtons.rend(); ++it)
        if (SameName(name, it->mName)) return &*it;
    return nullptr;
}

// tag is the integer identifier assigned to the desired button.
const AnimButton* ButtonGroup::FindButtonByTag(int tag) const {
    for (auto& button : mButtons) if (button.mTag == tag) return &button;
    return nullptr;
}

// layout is the owning layout whose button should be returned.
AnimButton* ButtonGroup::FindButtonByLayout(const Layout* layout) {
    for (auto& button : mButtons) if (button.GetLayout() == layout) return &button;
    return nullptr;
}

void ButtonGroup::ForceOffAll() { for (auto& button : mButtons) button.ForceOff(); }
void ButtonGroup::ForceOnAll() { for (auto& button : mButtons) button.ForceOn(); }
void ButtonGroup::ForceDownAll() { for (auto& button : mButtons) button.ForceDown(); }
void ButtonGroup::UpdateHitBoxAll() { for (auto& button : mButtons) button.UpdateHitBox(); }
void ButtonGroup::CancelAll() { for (auto& button : mButtons) button.Cancel(); }
// callback and argument are installed on every button in this group.
void ButtonGroup::SetStateChangeCallbackAll(AnimButton::StateChangeCallback callback, void* argument) {
    for (auto& button : mButtons) button.SetStateChangeCallback(callback, argument);
}

void ButtonGroup::FreeAll() {
    for (auto it = mButtons.begin(); it != mButtons.end();) {
        auto current = it++;
        mButtons.erase(current);
        AnimButton* button = &*current;
        button->~AnimButton();
        Layout::FreeMemory(button);
    }

    mSelected = nullptr;
    mDragging = nullptr;
}
}

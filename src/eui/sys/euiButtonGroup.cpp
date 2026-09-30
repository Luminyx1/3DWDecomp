#include <eui/euiButtonGroup.h>
#include <eui/euiAnimButton.h>
namespace eui {
ButtonGroup::ButtonGroup() : mHitButton(nullptr), mDownButton(nullptr), mDragButton(nullptr), mFlags(3) {}
ButtonGroup::~ButtonGroup() = default;
// touch selects touch input for the group and every currently registered button.
void ButtonGroup::SetTouchDevice(bool touch) {
    if (touch) mFlags |= 4;
    else mFlags &= ~4;

    for (auto& rControl : mButtons) static_cast<AnimButton&>(rControl).SetTouch(touch);
}

AnimButton* ButtonGroup::FindDownButton() {
    for (auto& rControl : mButtons) {
        auto& rButton = static_cast<AnimButton&>(rControl);

        if (rButton.IsDowning()) return &rButton;
    }

    return nullptr;
}

void ButtonGroup::ForceOffAll() {
    for (auto& rControl : mButtons) static_cast<AnimButton&>(rControl).ForceOff();
}

void ButtonGroup::ForceOnAll() {
    for (auto& rControl : mButtons) static_cast<AnimButton&>(rControl).ForceOn();
}

void ButtonGroup::ForceDownAll() {
    for (auto& rControl : mButtons) static_cast<AnimButton&>(rControl).ForceDown();
}

void ButtonGroup::CancelAll() {
    for (auto& rControl : mButtons) static_cast<AnimButton&>(rControl).Cancel();
}

// allow permits a held touch to activate each button without a fresh trigger.
void ButtonGroup::SetAllowNoTrigTouchAll(bool allow) {
    for (auto& rControl : mButtons) {
        auto& rButton = static_cast<AnimButton&>(rControl);

        if (allow) rButton.mFlags |= 0x100;
        else rButton.mFlags &= ~0x100;
    }
}

// enabled allows a press to begin when a touch first enters each button.
void ButtonGroup::SetDownWithTouchOnAll(bool enabled) {
    for (auto& rControl : mButtons) {
        auto& rButton = static_cast<AnimButton&>(rControl);

        if (enabled) rButton.mFlags |= 0x200;
        else rButton.mFlags &= ~0x200;
    }
}
}

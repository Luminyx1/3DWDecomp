#include <eui/euiButtonGroup.h>
#include <eui/euiAnimButton.h>
namespace eui {
ButtonGroup::ButtonGroup() : mHitButton(nullptr), mDownButton(nullptr), mDragButton(nullptr), mFlags(3) {}
ButtonGroup::~ButtonGroup() = default;
// touch selects touch input for the group and every currently registered button.
void ButtonGroup::SetTouchDevice(bool touch) {
    if (touch) {
        mFlags |= 4;
    } else {
        mFlags &= ~4;
    }

    ForEachButton([touch](AnimButton& rButton) { rButton.SetTouch(touch); });
}

AnimButton* ButtonGroup::FindDownButton() {
    for (auto& rControl : mButtons) {
        auto& rButton = static_cast<AnimButton&>(rControl);

        if (rButton.IsDowning()) {
            return &rButton;
        }
    }

    return nullptr;
}

void ButtonGroup::ForceOffAll() {
    ForEachButton([](AnimButton& rButton) { rButton.ForceOff(); });
}

void ButtonGroup::ForceOnAll() {
    ForEachButton([](AnimButton& rButton) { rButton.ForceOn(); });
}

void ButtonGroup::ForceDownAll() {
    ForEachButton([](AnimButton& rButton) { rButton.ForceDown(); });
}

void ButtonGroup::CancelAll() {
    ForEachButton([](AnimButton& rButton) { rButton.Cancel(); });
}

// allow permits a held touch to activate each button without a fresh trigger.
void ButtonGroup::SetAllowNoTrigTouchAll(bool allow) {
    ForEachButton([allow](AnimButton& rButton) { rButton.SetAllowNoTrigTouch(allow); });
}

// enabled allows a press to begin when a touch first enters each button.
void ButtonGroup::SetDownWithTouchOnAll(bool enabled) {
    ForEachButton([enabled](AnimButton& rButton) { rButton.SetDownWithTouchOn(enabled); });
}
}

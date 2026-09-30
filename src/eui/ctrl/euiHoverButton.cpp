#include <eui/euiHoverButton.h>

namespace eui {
const char* HoverButton::getClassName() const { return "HoverButton"; }

// A touch press ends the hover state; cursor activation does not press this button.
void HoverButton::Down() {
    if (mFlags & 0x40) Off();
}
}

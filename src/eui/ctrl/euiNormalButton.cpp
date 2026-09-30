#include <eui/euiNormalButton.h>

namespace eui {

const char* NormalButton::getClassName() const { return "NormalButton"; }

// rOther supplies button properties; pLayout owns the clone; pHeap holds its animations.
NormalButton::NormalButton(const NormalButton& rOther, LayoutEx* pLayout, sead::Heap* pHeap) {
    CloneImpl_(rOther, pLayout, pHeap);
}

}  // namespace eui

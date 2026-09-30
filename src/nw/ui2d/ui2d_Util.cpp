#include <nn/ui2d/ui2d_AnimTransform.h>
#include <nn/ui2d/ui2d_Layout.h>
#include <cstring>
namespace nn::ui2d {
// animation binds group panes; enabled controls whether its values are applied.
void BindAnimation(AnimTransform* animation, Group* group, bool enabled) { animation->BindGroup(group); animation->SetEnabled(enabled); }
// animation is detached from the panes in group.
void UnbindAnimation(AnimTransform* animation, Group* group) { animation->UnbindGroup(group); }
// pane is the current node in a depth-first traversal; returns its successor.
Pane* GetNextPane(Pane* pane) {
    auto* child = pane->m_Children.GetNext();

    if (child != &pane->m_Children) return reinterpret_cast<Pane*>(reinterpret_cast<char*>(child) - 8);

    while (pane->mParent) {
        auto* next = pane->m_Link.GetNext();
        auto* parent = pane->mParent;

        if (next != &parent->m_Children) return reinterpret_cast<Pane*>(reinterpret_cast<char*>(next) - 8);
        pane = parent;
    }

    return nullptr;
}

namespace detail {
// value is clamped in place to the inclusive interval from minimum to maximum.
void ClampValue(float& value, float minimum, float maximum) {
    if (value < minimum) value = minimum;
    else if (value > maximum) value = maximum;
}

// source is a terminated string copied into layout-allocated storage.
char* AllocateAndCopyString(const char* source) {
    size_t size = std::strlen(source) + 1;
    return std::strncpy(static_cast<char*>(Layout::AllocateMemory(size)), source, size);
}
}
}

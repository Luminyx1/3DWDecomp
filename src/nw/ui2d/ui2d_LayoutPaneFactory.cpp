#include <nn/ui2d/ui2d_LayoutPaneFactory.h>
#include <nn/ui2d/ui2d_Pane.h>
namespace nn::ui2d {
// source is the tree to copy; device and layout own its resources;
// context tracks resource sharing throughout the recursive clone.
Pane* LayoutPaneFactory::ClonePaneTree(const Pane* source, nn::gfx::Device* device, Layout* layout, detail::BuildPaneTreeContext* context) {
    Pane* clone = ClonePaneImpl_(source, device, layout, context);

    for (auto* node = source->m_Children.GetNext(); node != &source->m_Children; node = node->GetNext()) {
        auto* child = reinterpret_cast<const Pane*>(reinterpret_cast<const char*>(node) - 8);
        clone->AppendChild(ClonePaneTree(child, device, layout, context));
    }

    return clone;
}
}

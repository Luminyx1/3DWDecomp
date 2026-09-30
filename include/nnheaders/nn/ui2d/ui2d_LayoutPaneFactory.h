#pragma once

#include <nn/gfx/gfx_Types.h>

namespace nn::ui2d {
class Pane;
class Layout;
struct BuildResultInformation;
struct BuildArgSet;
namespace detail { struct BuildPaneTreeContext; }

class LayoutPaneFactory {
public:
    virtual ~LayoutPaneFactory();
    virtual Pane* BuildPaneObj(BuildResultInformation*, nn::gfx::Device*, u32,
                               const void*, const void*, const BuildArgSet&);
    virtual Pane* ClonePaneTree(const Pane*, nn::gfx::Device*, Layout*, detail::BuildPaneTreeContext*);
    virtual Pane* ClonePaneTreeWithPartsLayout(const Pane*, Layout*, nn::gfx::Device*, Layout*,
                                              detail::BuildPaneTreeContext*);
    virtual Pane* ClonePaneImpl_(const Pane*, nn::gfx::Device*, Layout*, detail::BuildPaneTreeContext*);
};

extern LayoutPaneFactory g_DefaultLayoutPaneFactory;
}

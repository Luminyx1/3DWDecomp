#include <nn/ui2d/ui2d_Bounding.h>

namespace nn::ui2d {

// pResource supplies the pane properties; pOverride is unused for bounding panes;
// rArgs supplies the owning layout and build context.
Bounding::Bounding(const ResBounding* pResource, const ResBounding* pOverride,
                   const BuildArgSet& rArgs)
    : Pane(nullptr, nullptr, reinterpret_cast<const ResPane*>(pResource), rArgs) {}

// rOther supplies the pane properties; its child tree is not copied.
Bounding::Bounding(const Bounding& rOther) : Pane(rOther) {}

Bounding::~Bounding() = default;

// rDrawInfo and rCommands are unused because bounding panes have no drawable content.
void Bounding::DrawSelf(DrawInfo& rDrawInfo, nn::gfx::CommandBuffer& rCommands) {}

// rOther is unused: bounding panes have no additional state to compare.
bool Bounding::CompareCopiedInstanceTest(const Bounding& rOther) const { return true; }

}  // namespace nn::ui2d

#include <eui/euiRootPane.h>

#include <nn/ui2d/ui2d_BuildArgSet.h>

namespace eui {

/** @brief Creates the root pane for a layout. */
RootPane::RootPane(LayoutEx* pLayout) : m_pLayout(pLayout) {}

/** @brief Builds a root pane using the layout in the build arguments. */
RootPane::RootPane(const nn::ui2d::ResPane* pResource, const nn::ui2d::BuildArgSet& rArgs)
    : Pane(pResource, rArgs), m_pLayout(reinterpret_cast<LayoutEx*>(rArgs.m_pLayout)) {}

/** @brief Copies the root pane into another layout. */
RootPane::RootPane(const RootPane& rOther, LayoutEx* pLayout)
    : Pane(rOther), m_pLayout(pLayout) {}

/** @brief Destroys the layout root pane. */
RootPane::~RootPane() = default;

static_assert(sizeof(RootPane) == 0xe0, "RootPane size");

}  // namespace eui

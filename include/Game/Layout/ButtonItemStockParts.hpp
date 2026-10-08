#pragma once

#include "Library/Layout/LayoutActor.hpp"

namespace al {
class LayoutInitInfo;
}  // namespace al

class ProjectItemDirector;

/**
 * @brief Item-stock button parts layout.
 * @note Minimal declaration: only what the reconstructed code uses is declared so far.
 */
class ButtonItemStockParts : public al::LayoutActor {
public:
    ButtonItemStockParts(const al::LayoutInitInfo& rInfo, const char* pName, const char* pPaneName,
                         al::LayoutActor* pParent, ProjectItemDirector* pItemDirector,
                         bool isUnknown);

private:
    unsigned char _padding[0x150 - 0x128];
};

static_assert(sizeof(ButtonItemStockParts) == 0x150);

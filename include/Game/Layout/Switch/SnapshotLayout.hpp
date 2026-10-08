#pragma once

#include <basis/seadTypes.h>

#include "Library/Layout/LayoutActor.hpp"

namespace al {
class LayoutInitInfo;
}  // namespace al

namespace rc {
class StampDirector;
}  // namespace rc

/**
 * @brief Layout of the snapshot mode.
 * @note Minimal declaration: only what the reconstructed code uses is declared so far.
 */
class SnapshotLayout : public al::LayoutActor {
public:
    SnapshotLayout(const al::LayoutInitInfo& rInfo, rc::StampDirector* pStampDirector);

private:
    u8 _121[0x148 - 0x121];
};

static_assert(sizeof(SnapshotLayout) == 0x148);

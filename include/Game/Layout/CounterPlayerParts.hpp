#pragma once

#include "Library/Layout/LayoutActor.hpp"

namespace al {
class LayoutInitInfo;
}  // namespace al

class PlayerAliveWatcher;

/**
 * @brief Player-count (lives) counter parts layout.
 * @note Minimal declaration: only what the reconstructed code uses is declared so far.
 */
class CounterPlayerParts : public al::LayoutActor {
public:
    CounterPlayerParts(const al::LayoutInitInfo& rInfo, const char* pName, const char* pPaneName,
                       al::LayoutActor* pParent, const PlayerAliveWatcher* pWatcher,
                       bool isUnknown);

private:
    unsigned char _padding[0x148 - 0x128];
};

static_assert(sizeof(CounterPlayerParts) == 0x148);

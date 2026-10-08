#pragma once

#include <basis/seadTypes.h>
#include "Library/Layout/LayoutActor.hpp"

namespace al {
class LayoutInitInfo;
}

/**
 * @brief Controller button guide shown from the pause menu.
 * @note Only the members used by already-decompiled callers are declared.
 */
class ControlGuide : public al::LayoutActor {
public:
    ControlGuide(const al::LayoutInitInfo& rInfo, bool isSingleMode);

    void appearWithPort(s32 port);
    bool isEnding();

    /** @brief Requests the guide to close on its next update. */
    void requestEnd() { mIsRequestEnd = true; }

private:
    u8 mUnreconstructed121[0x1f];
    bool mIsRequestEnd;
    u8 mUnreconstructed141[0x17];
};
static_assert(sizeof(ControlGuide) == 0x158);

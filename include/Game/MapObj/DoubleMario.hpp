#pragma once

#include "Library/LiveActor/LiveActor.hpp"

class ItemBubble;

class DoubleMario : public al::LiveActor {
public:
    static s32 calcAppearDoubleMarioNum(const al::ActorInitInfo& rInfo) { return 1; }
    explicit DoubleMario(const char* pName, ItemBubble* = nullptr, bool = false);

private:
    u8 mUnreconstructed[0x24];
};
static_assert(sizeof(DoubleMario) == 0x168);

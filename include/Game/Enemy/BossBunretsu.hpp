#pragma once

#include "Library/LiveActor/LiveActor.hpp"

class BossBunretsu : public al::LiveActor {
public:
    static s32 calcAppearDoubleMarioNum(const al::ActorInitInfo& rInfo);
    explicit BossBunretsu(const char* pName);

private:
    u8 mUnreconstructed[0xac];
};
static_assert(sizeof(BossBunretsu) == 0x1f0);

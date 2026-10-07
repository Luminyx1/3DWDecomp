#pragma once

#include "Library/LiveActor/LiveActor.hpp"

class KuriboTower : public al::LiveActor {
public:
    static s32 calcAppearDoubleMarioNum(const al::ActorInitInfo& rInfo);
    explicit KuriboTower(const char* pName);

private:
    u8 mUnreconstructed[0x74];
};
static_assert(sizeof(KuriboTower) == 0x1b8);

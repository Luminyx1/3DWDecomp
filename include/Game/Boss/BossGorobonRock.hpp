#pragma once

#include <basis/seadTypes.h>

#include "Library/LiveActor/LiveActor.hpp"

class BossGorobonRock : public al::LiveActor {
public:
    explicit BossGorobonRock(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(BossGorobonRock) == 0x158);

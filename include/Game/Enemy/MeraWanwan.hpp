#pragma once

#include <basis/seadTypes.h>

#include "Library/LiveActor/LiveActor.hpp"

class MeraWanwan : public al::LiveActor {
public:
    explicit MeraWanwan(const char* pName);

    void setFindDistance(f32 distance);
    void appearByBossGorobon();
    void forceDead();

private:
    u8 _144[0x85];

public:
    bool _1C9;

private:
    u8 _1CA[0x26];
};
static_assert(sizeof(MeraWanwan) == 0x1f0);

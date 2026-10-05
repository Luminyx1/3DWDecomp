#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class CoinBlow : public al::LiveActor {
public:
    explicit CoinBlow(const char*);
    void setOffCollide(int);
    void setHideModel(int);
    void setLifeTime(int frames) { mLifeTime = frames; }
private:
    int mUnknown144;
    int mLifeTime;
    u8 mUnreconstructed14C[0x34];
};
static_assert(sizeof(CoinBlow) == 0x180);

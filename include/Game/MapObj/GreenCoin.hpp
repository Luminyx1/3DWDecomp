#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class GreenRing;
class GreenCoin : public al::LiveActor {
public:
    GreenCoin(const char*);
    int getCountDownStep() const;
    static int getTimerFrame();
    void setHost(GreenRing* host) { mHost = host; }
private:
    u8 mUnreconstructed[0x188 - 0x144];
    GreenRing* mHost;
};
static_assert(sizeof(GreenCoin) == 0x190);

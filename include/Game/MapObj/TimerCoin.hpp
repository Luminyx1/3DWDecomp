#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class TimerCoin : public al::LiveActor {
public:
    TimerCoin(const char*);
    void setTimerFrame(int frames) { mTimerFrame = frames; }
    bool isCounted() const { return mIsCounted; }
    void setCounted() { mIsCounted = true; }
    al::HitSensor* getCollectSensor() const { return mCollectSensor; }
private:
    u8 mUnreconstructed144[0x158 - 0x144];
    al::HitSensor* mCollectSensor;
    u8 mUnreconstructed160[0x174 - 0x160];
    int mTimerFrame;
    bool mIsCounted;
    u8 mUnreconstructed179[0x188 - 0x179];
};

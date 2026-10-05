#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class IUseRedCoin;
class CoinRed : public al::LiveActor {
public:
    CoinRed(const char*, IUseRedCoin*);
    void disableCountdown();
    static int getTimerFrame();
    const sead::Vector3f& getItemDirection() const { return mItemDirection; }
    al::HitSensor* getCollector() const { return mCollector; }
    int getCollectedFrame() const { return mCollectedFrame; }
private:
    u8 mUnreconstructed[0x158 - 0x144];
    sead::Vector3f mItemDirection;
    u8 mUnreconstructed164[0x180 - 0x164];
    al::HitSensor* mCollector;
    int mCollectedFrame;
    u8 mUnreconstructed18C[0x198 - 0x18c];
};

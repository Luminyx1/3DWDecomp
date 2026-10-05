#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class ItemBubble;
class GreenStar : public al::LiveActor {
public:
    explicit GreenStar(const char*, ItemBubble* = nullptr, bool = false);
    bool isAcquiredInScene() const { return mIsAcquiredInScene; }
    al::HitSensor* getAcquirerSensor() const { return mAcquirerSensor; }
private:
    u8 mUnreconstructed144[0x3c];
    al::HitSensor* mAcquirerSensor;
    u8 mUnreconstructed188[0x25];
    bool mIsAcquiredInScene;
    u8 mUnreconstructed1ae[0x2a];
};
static_assert(sizeof(GreenStar) == 0x1d8);

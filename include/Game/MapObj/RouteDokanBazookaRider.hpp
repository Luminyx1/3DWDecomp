#pragma once

#include "Library/LiveActor/LiveActor.hpp"
#include <math/seadVector.h>

class IUsePlayerPuppet;
class RouteDokanBazooka;
class RouteDokanEntrance;

/**
 * @brief Carries one player through a RouteDokanBazooka and launches them out of it.
 */
class RouteDokanBazookaRider : public al::LiveActor {
public:
    RouteDokanBazookaRider(RouteDokanBazooka* pHost, const char* pName, s32 type, f32 gravity);

    void setMoveSpeed(f32 speed);
    void setOutVelocity(const sead::Vector3f& rVelocity);
    bool isActive(s32 playerIndex) const;
    void startBind(RouteDokanEntrance* pEntrance, const al::SensorMsg* pMsg,
                   al::HitSensor* pSender, al::HitSensor* pReceiver, bool isFlying);
    bool tryCancelBind(al::HitSensor* pSender);
    bool damage(al::HitSensor* pSender);
    bool isStateReady() const;
    bool isStateFlying() const;
    bool isEndBind() const;
    void shoot();

    IUsePlayerPuppet* getPuppet() const { return mPuppet; }

    s32 getPlayerIndex() const { return mPlayerIndex; }

    void setShootType(s32 shootType) { mShootType = shootType; }

    void setUpDir(const sead::Vector3f& rUpDir) { mUpDir = rUpDir; }

    void setShootFrame(s32 frame) { mShootFrame = frame; }

private:
    u8 _148[0x8];
    IUsePlayerPuppet* mPuppet;  // 0x150
    u8 _158[0x18];
    s32 mPlayerIndex;  // 0x170
    u8 _174[0x48];
    s32 mShootType;  // 0x1bc
    u8 _1c0[0x5c];
    sead::Vector3f mUpDir;  // 0x21c
    u8 _228[0x14];
    s32 mShootFrame;  // 0x23c
};

static_assert(sizeof(RouteDokanBazookaRider) == 0x240);

#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace al {
class LiveActor;
class SensorHitGroup;
class HitSensor;

enum class HitSensorType : u32 {
    Eye = 0,
    Player = 1,
    PlayerEye = 2,
    Npc = 3,
    Ride = 4,
    Enemy = 5,
    EnemyBody = 6,
    EnemyAttack = 7,
    KillerMagnum = 8,
    Dossun = 9,
    EnemySimple = 10,
    MapObj = 11,
    MapObjSimple = 12,
    Bindable = 13,
    CollisionParts = 14,
    KickKoura = 15,
    PlayerFireBall = 16,
    WooGanSandBody = 17,
    HoldObj = 18,
    BindableGigaBell = 19,
    BindableGoal = 20,
    BindableAllPlayer = 21,
    BindableBubbleOutScreen = 22,
    BindableKoura = 23,
    BindableRouteDokan = 24,
    BindableBubblePadInput = 25,
    MultiPlayer = 26,
    KoopaJr = 27,
    CutsceneStart = 28,
    NpcAvoid = 29,
    BindableNpc = 30,
    BindableGoalItem = 31,
};

class SensorSortCmpFuncBase {
public:
    virtual bool compare(HitSensor* pA, HitSensor* pB) const = 0;
};

class SensorSortCmpFunc {
public:
    SensorSortCmpFunc(const SensorSortCmpFuncBase* pFunc) : mFunc(pFunc) {}

    bool operator()(HitSensor* pA, HitSensor* pB) { return mFunc->compare(pA, pB); }

    const SensorSortCmpFuncBase* mFunc;
};

class HitSensor {
public:
    HitSensor(LiveActor* pHost, const char* pName, u32 type, f32 radius, u16 maxSensors,
              const sead::Vector3f* pFollowPos, const sead::Matrix34f* pFollowMtx,
              const sead::Vector3f& rOffset);

    void trySensorSort();
    void setFollowPosPtr(const sead::Vector3f* pFollowPos);
    void setFollowMtxPtr(const sead::Matrix34f* pFollowMtx);
    void validate();
    void invalidate();
    void validateBySystem();
    void invalidateBySystem();
    void update();
    void addHitSensor(HitSensor* pSensor);
    void setTime();

    const char* getName() const { return mName; }
    const sead::Vector3f& getPos() const { return mPos; }
    f32 getRadius() const { return mRadius; }
    LiveActor* getHost() const { return mHostActor; }
    HitSensorType getType() const { return mSensorType; }
    bool isType(HitSensorType type) const { return mSensorType == type; }
    const sead::Vector3f& getFollowPosOffset() const { return mFollowPosOffset; }
    void setRadius(f32 radius) { mRadius = radius; }
    void setHitGroup(SensorHitGroup* pGroup) { mHitGroup = pGroup; }
    void clearSensors() { mNumSensors = 0; }

    const char* mName;
    HitSensorType mSensorType;
    sead::Vector3f mPos = {0.0f, 0.0f, 0.0f};
    f32 mRadius;
    u16 mMaxSensors;
    u16 mNumSensors = 0;
    HitSensor** mSensors = nullptr;
    SensorSortCmpFunc* mSortFunc = nullptr;
    SensorHitGroup* mHitGroup = nullptr;
    bool mIsValidBySystem = false;
    bool mIsValid = true;
    LiveActor* mHostActor;
    const sead::Vector3f* mFollowPos;
    const sead::Matrix34f* mFollowMtx;
    sead::Vector3f mFollowPosOffset;
    s64 mTime = -1;
};

}  // namespace al

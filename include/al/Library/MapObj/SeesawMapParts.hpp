#pragma once

#include <math/seadQuat.h>

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class SeesawMapParts : public LiveActor {
public:
    SeesawMapParts(const char* pName);

    void init(const ActorInitInfo& rInfo) override;
    bool receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) override;

    void exeWait();

    sead::Quatf mStartQuat = sead::Quatf::unit;
    sead::Vector3f mSide = sead::Vector3f::ex;
    sead::Vector3f mFront = sead::Vector3f::ez;
    f32 mRotateDegree = 0.0f;
    f32 mRotateSpeed = 0.0f;
    f32 mRotateAccelOn = 0.025f;
    f32 mRotateAccelOff = 0.0125f;
    f32 mMaxDegree = 45.0f;
    f32 mWeight = 0.0f;
    s32 mRemainingAccelOnFrames = 0;
};
}  // namespace al

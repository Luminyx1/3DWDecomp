#pragma once

#include <math/seadQuat.h>

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class WobbleMapParts : public LiveActor {
public:
    WobbleMapParts(const char* pName);

    void init(const ActorInitInfo& rInfo) override;
    bool receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) override;

    void exeWait();
    void updateMove();
    void exeMove();
    void exeAssistStop();

    sead::Quatf mInitialQuat = sead::Quatf::unit;
    sead::Vector3f mInitialUp = sead::Vector3f::ey;
    sead::Quatf mCurrentQuat = sead::Quatf::unit;
    sead::Vector3f mMoment = sead::Vector3f::zero;
    sead::Vector3f mTargetUp = sead::Vector3f::ey;
    sead::Vector3f _188;
    f32 mMaxRotate = 10.0f;
    f32 mTiltSpeed = 0.0f;
    f32 mRotateSoundScale = 1.0f;
    s32 mAssistStopTimer = 0;
    bool mIsStop = false;
};
}  // namespace al

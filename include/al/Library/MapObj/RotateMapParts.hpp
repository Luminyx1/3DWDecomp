#pragma once

#include <math/seadQuat.h>

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class RotateMapParts : public LiveActor {
public:
    RotateMapParts(const char* pName);

    void init(const ActorInitInfo& rInfo) override;
    void appear() override;
    void kill() override;
    bool receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) override;

    void start();
    void exeStandBy();
    void exeRotate();
    void exeAssistStop();
    void exeAssistStopSync();

    s32 mRotateAxis = 1;
    f32 mRotateSpeed = 100.0f;
    s32 mAssistTimer = 0;
    bool mIsSupportFreezeSync = false;
    sead::Vector3f mStartTrans = sead::Vector3f::zero;
    sead::Quatf mStartQuat;
    bool mIsSingleMode = false;
    bool mIsTriggerEffectOnAngle = false;
    f32 mEffectAngle = 0.0f;
    f32 mEffectTriggerAngle = 0.0f;
};
}  // namespace al

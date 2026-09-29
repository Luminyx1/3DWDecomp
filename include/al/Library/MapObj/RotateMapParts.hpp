#pragma once

#include <math/seadQuat.h>

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class RotateMapParts : public LiveActor {
public:
    RotateMapParts(const char*);

    void init(const ActorInitInfo&) override;
    void appear() override;
    void kill() override;
    bool receiveMsg(const SensorMsg* pMsg, HitSensor* pSelf, HitSensor* pOther) override;

    void start();
    void exeStandBy();
    void exeRotate();
    void exeAssistStop();
    void exeAssistStopSync();

    s32 mRotateAxis = 1;                           // _144
    f32 mRotateSpeed = 100.0f;                     // _148
    s32 mAssistTimer = 0;                          // _14c
    bool mIsSupportFreezeSync = false;             // _150
    sead::Vector3f mStartTrans = sead::Vector3f::zero;  // _154
    sead::Quatf mStartQuat;                        // _160
    bool mIsResetOnAppear = false;                 // _170
    bool mIsTriggerEffectOnAngle = false;          // _171
    f32 mAngle = 0.0f;                             // _174
    f32 mEffectTriggerAngle = 0.0f;                // _178
};
}  // namespace al

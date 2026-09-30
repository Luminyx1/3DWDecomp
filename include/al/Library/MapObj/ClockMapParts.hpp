#pragma once

#include <math/seadQuat.h>

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class ClockMapParts : public LiveActor {
public:
    ClockMapParts(const char* pName);

    void init(const ActorInitInfo& rInfo) override;
    void startDemoActor(s32 demoType) override;
    void endDemoActor(s32 demoType) override;
    bool receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) override;

    void start();
    void setRestartNerve();
    void setRotateStartNerve();
    void exeStandBy();
    void exeDelay();
    void exeRotateSign();
    void exeRotate();
    void exeWait();
    void exeAssistStop();
    void exeAssistStopSync();
    void exeAssistStopEndWait();

    sead::Quatf mQuat = sead::Quatf::unit;
    s32 mRotateAxis = 0;
    f32 mClockAngle = 0.0f;
    s32 mCurrentStep = 0;
    s32 mDelayTime = 0;
    s32 mRotateTime = 30;
    s32 mWaitTime = 30;
    s32 mRotateSignTime = 0;
    s32 mRotateTimer = 30;
    s32 mActiveTimer;
    s32 mTimer = 0;
    s32 mAssistStopTimer;
    bool mIsNoIntroUpdate = false;
    bool mIsDemo = false;
};
}  // namespace al

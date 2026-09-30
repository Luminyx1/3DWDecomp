#pragma once

#include <math/seadQuat.h>

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class SwitchKeepOnAreaGroup;
class SwitchOnAreaGroup;

class WheelMapParts : public LiveActor {
public:
    WheelMapParts(const char* pName);

    void init(const ActorInitInfo& rInfo) override;
    void appear() override;
    void kill() override;
    bool receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) override;
    void control() override;

    void exeWait();
    void exeAssistStop();

    SwitchKeepOnAreaGroup* mSwitchKeepOnAreaGroup = nullptr;
    SwitchOnAreaGroup* mSwitchOnAreaGroup = nullptr;
    sead::Quatf mInitialQuat = sead::Quatf::unit;
    sead::Vector3f mMoveDir = sead::Vector3f::ez;
    s32 mRotateAxis = 0;
    f32 mMoveEndDegree = 360.0f;
    f32 mWheelAngle = 0.0f;
    f32 mDeltaAngle = 0.0f;
    f32 mRotateAccel = 20.0f;
    f32 mRotateWidth = 0.0f;
    f32 mNoRotateWidth = 25.0f;
    s32 mAssistStopTimer = 0;
    bool mIsRailPlusDir = true;
    bool mIsResetOnKill = false;
};
}  // namespace al

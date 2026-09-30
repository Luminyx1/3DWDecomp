#pragma once

#include <math/seadMatrix.h>
#include <math/seadQuat.h>

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class EffectMtxSetter;
class KeyPoseKeeper;
class SwitchKeepOnAreaGroup;
class SwitchOnAreaGroup;

class KeyMoveMapParts : public LiveActor {
public:
    KeyMoveMapParts(const char* pName);

    void init(const ActorInitInfo& rInfo) override;
    void initAfterPlacement() override;
    void appear() override;
    bool receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) override;
    void control() override;
    virtual void exeWait();
    virtual void exeMove();
    virtual void _startUp();
    virtual void _startDown();

    const char* getMoveSeName(s32 index);
    void start();
    void reverse();
    void killLights();
    void stop();
    sead::Vector3f getGroundPos();
    void appearAndSetStart();
    void exeStandBy();
    void exeDelay();
    void exeWaitSign();
    void exeMoveSign();
    void exeStopSign();
    void exeStop();

    KeyPoseKeeper* getKeyPoseKeeper() const { return mKeyPoseKeeper; }

    EffectMtxSetter* mEffectMtxSetter = nullptr;
    sead::Matrix34f mBaseEffectMtx = sead::Matrix34f::ident;
    f32 mGroundCheckOffset = 1600.0f;
    sead::Quatf mBaseQuat = sead::Quatf::unit;
    KeyPoseKeeper* mKeyPoseKeeper = nullptr;
    SwitchKeepOnAreaGroup* mSwitchKeepOnAreaGroup = nullptr;
    SwitchOnAreaGroup* mSwitchOnAreaGroup = nullptr;
    sead::Vector3f mClippingOffset = sead::Vector3f::zero;
    s32 mKeyMoveWaitTime = 30;
    s32 mKeyMoveMoveTime = 0;
    s32 mDelayTime = 0;
    bool mIsFloorTouchStart = false;
    bool mIsStopKill = false;
    const char* mSeMoveName = nullptr;
    bool mIsCancelWithinWaitTime = false;
    bool mIsReverseWhenSwitchOff = false;
    bool mIsReversed = false;
    s32 mReverseTimer = -1;
    bool mIsIgnoreFirstWaitTime = false;
    bool mIsSingleMode;
};
}  // namespace al

#pragma once

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
template <class T>
class DeriveActorGroup;
class ConveyerKeyKeeper;
class ConveyerStep;

class ConveyerMapParts : public LiveActor {
public:
    ConveyerMapParts(const char* pName);

    void init(const ActorInitInfo& rInfo) override;
    void startClipped() override;
    void endClipped() override;
    bool receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) override;
    void control() override;

    void start();
    void stop();
    void exeStandBy();
    void exeMove();

    DeriveActorGroup<ConveyerStep>* mConveyerStepGroup = nullptr;
    ConveyerKeyKeeper* mConveyerKeyKeeper = nullptr;
    sead::Vector3f mClippingTrans = sead::Vector3f::zero;
    f32 mOffsetCoord = 0.0f;
    f32 mMoveSpeed = 5.0f;
    f32 mPartsInterval = 200.0f;
    f32 mMaxCoord = 0.0f;
    s32 mAddRideActiveFrames = 0;
    s32 mRideActiveFrames = 0;
    s32 mMaxRideActiveFrames = 30;
    bool mIsRideOnlyMove = false;
};
}  // namespace al

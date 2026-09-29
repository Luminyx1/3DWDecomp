#pragma once

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
template <class T>
class DeriveActorGroup;
class ConveyerKeyKeeper;
class ConveyerStep;

class ConveyerMapParts : public LiveActor {
public:
    ConveyerMapParts(const char*);

    void init(const ActorInitInfo&) override;
    bool receiveMsg(const SensorMsg* pMsg, HitSensor* pSelf, HitSensor* pOther) override;
    void control() override;
    void startClipped() override;
    void endClipped() override;

    void start();
    void stop();
    void exeStandBy();
    void exeMove();

    DeriveActorGroup<ConveyerStep>* mConveyerStepGroup = nullptr;  // _148
    ConveyerKeyKeeper* mConveyerKeyKeeper = nullptr;                // _150
    sead::Vector3f mClippingTrans = sead::Vector3f::zero;           // _158
    f32 mOffsetCoord = 0.0f;                                        // _164
    f32 mMoveSpeed = 5.0f;                                          // _168
    f32 mPartsInterval = 200.0f;                                    // _16c
    f32 mMaxCoord = 0.0f;                                           // _170
    s32 mAddRideActiveFrames = 0;                                   // _174
    s32 mRideActiveFrames = 0;                                      // _178
    s32 mMaxRideActiveFrames = 30;                                  // _17c
    bool mIsRideOnlyMove = false;                                   // _180
};
}  // namespace al

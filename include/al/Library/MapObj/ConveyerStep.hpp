#pragma once

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class ConveyerKeyKeeper;

class ConveyerStep : public LiveActor {
public:
    ConveyerStep(const char*);

    void init(const ActorInitInfo&) override;
    bool receiveMsg(const SensorMsg* pMsg, HitSensor* pSelf, HitSensor* pOther) override;

    void setHost(LiveActor*);
    void setConveyerKeyKeeper(const ConveyerKeyKeeper*, f32);
    void setTransByCoord(f32, bool);
    void setTransByCoord(f32, bool, bool);
    void setTransAndResetByCoord(f32);

    void exeWait();

    LiveActor* mHost = nullptr;                            // _148
    const ConveyerKeyKeeper* mConveyerKeyKeeper = nullptr;  // _150
    const char* mKeyHitReactionName = nullptr;             // _158
    const char* mActionName = nullptr;                     // _160
    f32 mCurrentCoord = 0.0f;                              // _168
    f32 mMaxCoord = 0.0f;                                  // _16c
};
}  // namespace al

#pragma once

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class ConveyerKeyKeeper;

class ConveyerStep : public LiveActor {
public:
    ConveyerStep(const char* pName);

    void init(const ActorInitInfo& rInfo) override;
    bool receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) override;

    void setHost(LiveActor* pHost);
    void setConveyerKeyKeeper(const ConveyerKeyKeeper* pConveyerKeyKeeper, f32 coord);
    void setTransByCoord(f32 coord, bool isForwards);
    void setTransByCoord(f32 coord, bool isForwards, bool isForceReset);
    void setTransAndResetByCoord(f32 coord);
    void exeWait();

    LiveActor* mHost = nullptr;
    const ConveyerKeyKeeper* mConveyerKeyKeeper = nullptr;
    const char* mKeyHitReactionName = nullptr;
    const char* mActionName = nullptr;
    f32 mCurrentCoord = 0.0f;
    f32 mMaxCoord = 0.0f;
};
}  // namespace al

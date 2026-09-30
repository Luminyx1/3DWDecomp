#pragma once

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class ChildStep : public LiveActor {
public:
    ChildStep(const char* pName, LiveActor* pParent);

    void init(const ActorInitInfo& rInfo) override;
    bool receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) override;

    void exeWait();

    LiveActor* mParent;
    sead::Vector3f mLocalTrans = sead::Vector3f::zero;
};

s32 calcChildStepCount(const ActorInitInfo& rInfo);
void tryInitSubActorKeeperChildStep(LiveActor* pActor, const ActorInitInfo& rInfo);
void createChildStep(const ActorInitInfo& rInfo, LiveActor* pParent, bool isSyncClipping);
}  // namespace al

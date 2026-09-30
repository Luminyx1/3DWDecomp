#pragma once

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class FallMapParts : public LiveActor {
public:
    FallMapParts(const char* pName);

    void init(const ActorInitInfo& rInfo) override;
    bool receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) override;
    virtual void switchAppear();
    virtual void switchKill();

    void init(const ActorInitInfo& rInfo, const char* pSuffix);
    void exeAppear();
    void exeWait();
    void exeFallSign();
    bool isEndFallSign() const;
    void exeFall();
    void exeEnd();

    sead::Vector3f mStartTrans = sead::Vector3f::zero;
    s32 mFallTime = 75;
    bool mIsStartFallSignAction = false;
};
}  // namespace al

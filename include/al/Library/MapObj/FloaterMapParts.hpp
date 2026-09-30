#pragma once

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class KeyPoseKeeper;

class FloaterMapParts : public LiveActor {
public:
    FloaterMapParts(const char* pName);

    void init(const ActorInitInfo& rInfo) override;
    bool receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) override;
    void control() override;

    void exeWait();
    void exeSink();
    void exeBack();

    KeyPoseKeeper* mKeyPoseKeeper = nullptr;
    f32 mCoord = 0.0f;
    f32 mMaxCoord = 0.0f;
    f32 mSinkSpeed = 5.0f;
    f32 mBackSpeed = 5.0f;
    s32 mSinkFrame = 0;
    s32 mSinkTime = 0;
    s32 mSinkKeepTime = 10;
    s32 mMaxAccelCount = 10;
    s32 mAccelCount = 0;
};
}  // namespace al

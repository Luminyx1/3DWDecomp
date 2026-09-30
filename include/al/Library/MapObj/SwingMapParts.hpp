#pragma once

#include <math/seadQuat.h>

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class SwingMovement;

class SwingMapParts : public LiveActor {
public:
    SwingMapParts(const char* pName);

    void init(const ActorInitInfo& rInfo) override;
    bool receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) override;
    void control() override;

    void start();
    void exeStandBy();
    void exeMoveRight();
    void exeMoveLeft();
    void exeStop();

    sead::Quatf mStartQuat = sead::Quatf::unit;
    SwingMovement* mSwingMovement = nullptr;
    s32 mRotateAxis = 0;
    bool mIsFloorTouchStart = false;
};
}  // namespace al

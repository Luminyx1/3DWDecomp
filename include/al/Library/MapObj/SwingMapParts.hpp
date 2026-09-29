#pragma once

#include <math/seadQuat.h>

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class SwingMovement;

class SwingMapParts : public LiveActor {
public:
    SwingMapParts(const char*);

    void init(const ActorInitInfo&) override;
    bool receiveMsg(const SensorMsg* pMsg, HitSensor* pSelf, HitSensor* pOther) override;
    void control() override;

    void start();
    void exeStandBy();
    void exeMoveRight();
    void exeMoveLeft();
    void exeStop();

    sead::Quatf mStartQuat = sead::Quatf::unit;  // _144
    SwingMovement* mSwingMovement = nullptr;     // _158
    s32 mRotateAxis = 0;                         // _160
    bool mIsFloorTouchStart = false;             // _164
};
}  // namespace al

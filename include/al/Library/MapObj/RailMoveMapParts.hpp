#pragma once

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class RailMoveMovement;
class SwitchKeepOnAreaGroup;
class SwitchOnAreaGroup;

class RailMoveMapParts : public LiveActor {
public:
    RailMoveMapParts(const char* pName);

    void init(const ActorInitInfo& rInfo) override;
    bool receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) override;
    void control() override;
    virtual void exeStandBy();
    virtual void exeMoveSign();
    virtual void exeMove();

    void start();
    void stop();

    SwitchKeepOnAreaGroup* mSwitchKeepOnAreaGroup = nullptr;
    SwitchOnAreaGroup* mSwitchOnAreaGroup = nullptr;
    sead::Vector3f mRailPos = {0.0f, 0.0f, 0.0f};
    bool mIsAlwaysUpdateCollMtx = false;
    RailMoveMovement* mRailMoveMovement;
};
}  // namespace al

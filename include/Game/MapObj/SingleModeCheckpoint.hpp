#pragma once
#include "Library/LiveActor/LiveActor.hpp"
namespace al { class PlacementId; }
class SingleModeCheckpoint : public al::LiveActor {
public:
    explicit SingleModeCheckpoint(const char*);
    ~SingleModeCheckpoint() override;
    void init(const al::ActorInitInfo&) override;
    void initAfterPlacement() override;
    void attackSensor(al::HitSensor*, al::HitSensor*) override;
    bool receiveMsg(const al::SensorMsg*, al::HitSensor*, al::HitSensor*) override;
    bool receiveMsgScreenPoint(const al::SensorMsg*, al::ScreenPointer*, al::ScreenPointTarget*) override;
    void exeBefore();
    void exeAfter();
    void exeGet();
    void exeShakeBefore();
    void exeShake();
    void exeShakeEnd();
    void setStateAfter();
    void setStateBefore();
private:
    al::ActorInitInfo* mPlayerInfo = nullptr;
    al::PlacementId* mPlacementId;
    int mCheckpointId = 0;
};
static_assert(sizeof(SingleModeCheckpoint) == 0x160);

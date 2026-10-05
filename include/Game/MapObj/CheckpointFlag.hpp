#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class ActorMicRumbler;
namespace al { class PlacementId; }
class CheckpointFlag : public al::LiveActor {
public:
    explicit CheckpointFlag(const char*);
    ~CheckpointFlag() override;
    void init(const al::ActorInitInfo&) override;
    void control() override;
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
private:
    al::ActorInitInfo* mPlayerInfo = nullptr;
    al::PlacementId* mPlacementId;
    ActorMicRumbler* mRumbler = nullptr;
};
static_assert(sizeof(CheckpointFlag) == 0x160);

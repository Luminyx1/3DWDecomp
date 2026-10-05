#pragma once
#include "Library/Nerve/NerveStateBase.hpp"
namespace al { class ActorInitInfo; class SensorMsg; class HitSensor; class ScreenPointer; class ScreenPointTarget; }
class BlockRouletteState : public al::ActorStateBase {
public:
    BlockRouletteState(al::LiveActor*, const al::ActorInitInfo&);
    void reset();
    bool receiveMsg(const al::SensorMsg*, al::HitSensor*, al::HitSensor*);
    bool receiveMsgScreenPoint(const al::SensorMsg*, al::ScreenPointer*, al::ScreenPointTarget*);
private:
    u8 mUnreconstructed[0x28];
};
static_assert(sizeof(BlockRouletteState) == 0x48);

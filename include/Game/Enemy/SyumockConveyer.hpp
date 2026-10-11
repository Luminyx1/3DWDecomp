#pragma once
#include "Library/LiveActor/LiveActor.hpp"
namespace al { class ConveyerKeyKeeper; }

class SyumockConveyer : public al::LiveActor {
public:
    SyumockConveyer(const char* pName);
    void init(const al::ActorInitInfo& rInfo) override;
    void attackSensor(al::HitSensor* pSelf, al::HitSensor* pOther) override;
    bool receiveMsg(const al::SensorMsg* pMsg, al::HitSensor* pOther, al::HitSensor* pSelf) override;
    bool receiveMsgScreenPoint(const al::SensorMsg* pMsg, al::ScreenPointer* pPointer,
                               al::ScreenPointTarget* pTarget) override;
    void control() override;
    void setConveyerKeyKeeper(const al::ConveyerKeyKeeper* pKeeper, f32 maxCoord);
    void setTransByCoord(f32 coord, bool forward, bool reset);
    void exeMove();
    void exeReaction();
    void exeSupportFreeze();
    void exeEnd();
private:
    u8 _144[0x3c];
};
static_assert(sizeof(SyumockConveyer) == 0x180);

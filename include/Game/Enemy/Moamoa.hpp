#pragma once
#include "Library/LiveActor/LiveActor.hpp"
#include <math/seadVector.h>
class MoamoaHolder;

class Moamoa : public al::LiveActor {
public:
    Moamoa(const char* pName, const MoamoaHolder* pHolder);
    void init(const al::ActorInitInfo& rInfo) override;
    void control() override;
    void attackSensor(al::HitSensor* pSelf, al::HitSensor* pOther) override;
    bool receiveMsg(const al::SensorMsg* pMsg, al::HitSensor* pOther, al::HitSensor* pSelf) override;
    bool receiveMsgScreenPoint(const al::SensorMsg* pMsg, al::ScreenPointer* pPointer,
                               al::ScreenPointTarget* pTarget) override;
    /** @brief Sets the offset within the holder's grid. @param rOffset Local placement offset. */
    void setOffset(const sead::Vector3f& rOffset) { mOffset.set(rOffset); }
private:
    u8 _144[0x14];
    sead::Vector3f mOffset;
};
static_assert(sizeof(Moamoa) == 0x168);

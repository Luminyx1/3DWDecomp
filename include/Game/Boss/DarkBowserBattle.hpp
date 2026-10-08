#pragma once

#include "Library/Nerve/NerveStateBase.hpp"

namespace al {
class ActorInitInfo;
class HitSensor;
class SensorMsg;
}  // namespace al

class DarkBowser;

/** @brief Fury Bowser's battle state, which drives all of his attacks. */
class DarkBowserBattle : public al::NerveStateBase {
public:
    DarkBowserBattle(DarkBowser* pDarkBowser, const al::ActorInitInfo& rInfo);

    virtual void attackSensor(al::HitSensor* pSelf, al::HitSensor* pOther);
    virtual bool receiveMsg(const al::SensorMsg* pMsg, al::HitSensor* pSelf,
                            al::HitSensor* pOther);

    void registerGuideFrameOut();
    void endCamera();

private:
    u8 _18[0xa0 - 0x18];
};
static_assert(sizeof(DarkBowserBattle) == 0xa0);

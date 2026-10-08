#pragma once

#include <basis/seadTypes.h>

#include "Library/Nerve/NerveStateBase.hpp"

namespace al {
class ActorInitInfo;
class HitSensor;
}  // namespace al

class DarkBowser;

/**
 * @brief Fury Bowser's mouth laser attack state.
 * @note Only what reconstructed code needs is declared so far.
 */
class DarkBowserLaser : public al::NerveStateBase {
public:
    DarkBowserLaser(DarkBowser* pHost, const al::ActorInitInfo& rInfo);

    void attackSensor(al::HitSensor* pSelf, al::HitSensor* pOther);
    void updateLaserLight();

    /**
     * @brief Sets the attack level (how aggressive the attack is).
     * @param level Attack level.
     */
    void setLevel(s32 level) { mLevel = level; }

private:
    u8 _11[0x30 - 0x11];  // starts in the tail padding of al::NerveStateBase
    s32 mLevel;  // 0x30
    u8 _34[0x140 - 0x34];
};
static_assert(sizeof(DarkBowserLaser) == 0x140);

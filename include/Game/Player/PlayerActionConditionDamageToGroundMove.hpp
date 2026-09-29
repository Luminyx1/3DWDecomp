#pragma once

#include <basis/seadTypes.h>

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerCollision;
class IUsePlayerInput;
class IUsePlayerIsDamageEnd;
class PlayerConstParam;

/// Holds when the player tilts the stick on the ground after the damage cancel time.
class PlayerActionConditionDamageToGroundMove : public PlayerActionCondition {
public:
    PlayerActionConditionDamageToGroundMove(const IUsePlayerCollision*, const IUsePlayerInput*,
                                            const IUsePlayerIsDamageEnd*, const PlayerConstParam*);

    bool check() override;
    void setup() override;

private:
    u32 mFrame = 0;                          // 0x8
    const IUsePlayerCollision* mCollision;   // 0x10
    const IUsePlayerInput* mInput;           // 0x18
    const IUsePlayerIsDamageEnd* mDamageEnd;  // 0x20
    const PlayerConstParam* mConstParam;     // 0x28
};

#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerDamageInvalidCheck;
class PlayerTrigger;

/// Holds on the frame the player gets hurt, unless damage is blocked.
class PlayerActionConditionToDamage : public PlayerActionCondition {
public:
    PlayerActionConditionToDamage(const PlayerTrigger*, const IUsePlayerDamageInvalidCheck*);

    bool check() override;

private:
    const PlayerTrigger* mTrigger;                            // 0x8
    const IUsePlayerDamageInvalidCheck* mDamageInvalidCheck;  // 0x10
};

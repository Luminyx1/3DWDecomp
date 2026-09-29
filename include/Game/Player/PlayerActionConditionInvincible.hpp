#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerInvincibleCheck;

/// Holds while the player is invincible.
class PlayerActionConditionInvincible : public PlayerActionCondition {
public:
    PlayerActionConditionInvincible(const IUsePlayerInvincibleCheck*);

    bool check() override;

private:
    const IUsePlayerInvincibleCheck* mInvincibleCheck;  // 0x8
};

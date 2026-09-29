#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerCollision;
class IUsePlayerInput;

/// Holds when a jump is input while the player's back is to a wall.
class PlayerActionConditionWallToWallJump : public PlayerActionCondition {
public:
    PlayerActionConditionWallToWallJump(const IUsePlayerInput*, const IUsePlayerCollision*);

    bool check() override;

private:
    const IUsePlayerInput* mInput;  // 0x8
    const IUsePlayerCollision* mCollision;  // 0x10
};

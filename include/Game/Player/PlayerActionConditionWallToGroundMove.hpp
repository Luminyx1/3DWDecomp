#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerCollision;

/// Holds once the player clinging to a wall touches the floor.
class PlayerActionConditionWallToGroundMove : public PlayerActionCondition {
public:
    PlayerActionConditionWallToGroundMove(const IUsePlayerCollision*);

    bool check() override;

private:
    const IUsePlayerCollision* mCollision;  // 0x8
};

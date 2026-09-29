#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerCollision;

/// Holds while the player's head touches a wall in front.
class PlayerActionConditionHeadOnFrontWall : public PlayerActionCondition {
public:
    PlayerActionConditionHeadOnFrontWall(const IUsePlayerCollision*);

    bool check() override;

private:
    const IUsePlayerCollision* mCollision;  // 0x8
};

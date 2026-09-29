#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerCollision;

/// Holds while the player touches a wall in front.
class PlayerActionConditionFrontWall : public PlayerActionCondition {
public:
    PlayerActionConditionFrontWall(const IUsePlayerCollision*);

    bool check() override;

private:
    const IUsePlayerCollision* mCollision;  // 0x8
};

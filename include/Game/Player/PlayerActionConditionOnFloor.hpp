#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerCollision;

/// Holds while the player stands on the floor.
class PlayerActionConditionOnFloor : public PlayerActionCondition {
public:
    PlayerActionConditionOnFloor(const IUsePlayerCollision* pCollision) : mCollision(pCollision) {}

    bool check() override;

private:
    const IUsePlayerCollision* mCollision;  // 0x8
};

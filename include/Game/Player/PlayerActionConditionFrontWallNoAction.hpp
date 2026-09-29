#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerCollision;

/// Holds while the wall in front has the "NoAction" wall code (no wall jumps or slides).
class PlayerActionConditionFrontWallNoAction : public PlayerActionCondition {
public:
    PlayerActionConditionFrontWallNoAction(const IUsePlayerCollision*);

    bool check() override;

private:
    const IUsePlayerCollision* mCollision;  // 0x8
};

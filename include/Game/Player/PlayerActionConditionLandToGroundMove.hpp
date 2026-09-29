#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerActionCancelable;
class IUsePlayerCollision;
class IUsePlayerInput;

/// Holds when the landing can be cancelled into walking.
class PlayerActionConditionLandToGroundMove : public PlayerActionCondition {
public:
    PlayerActionConditionLandToGroundMove(const IUsePlayerInput*, const IUsePlayerCollision*, const IUsePlayerActionCancelable*);

    bool check() override;

private:
    const IUsePlayerInput* mInput;  // 0x8
    const IUsePlayerCollision* mCollision;  // 0x10
    const IUsePlayerActionCancelable* mCancelable;  // 0x18
};

#pragma once

#include "Player/PlayerActionCondition.hpp"

class PlayerTrigger;

/// Holds on the frame the player tramples something.
class PlayerActionConditionToTrampleJump : public PlayerActionCondition {
public:
    PlayerActionConditionToTrampleJump(const PlayerTrigger* pTrigger) : mTrigger(pTrigger) {}

    bool check() override;

private:
    const PlayerTrigger* mTrigger;  // 0x8
};

#pragma once

#include "Player/PlayerActionCondition.hpp"

class PlayerSpinJumpChecker;

/// Holds when a spin is input.
class PlayerActionConditionSpin : public PlayerActionCondition {
public:
    PlayerActionConditionSpin(const PlayerSpinJumpChecker*);

    bool check() override;

private:
    const PlayerSpinJumpChecker* mSpinJumpChecker;  // 0x8
};

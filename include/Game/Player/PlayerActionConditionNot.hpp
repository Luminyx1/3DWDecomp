#pragma once

#include "Player/PlayerActionCondition.hpp"

/// Inverts another condition.
class PlayerActionConditionNot : public PlayerActionCondition {
public:
    PlayerActionConditionNot(PlayerActionCondition*);

    bool check() override;
    void setup() override;

private:
    PlayerActionCondition* mCondition;  // 0x8
};

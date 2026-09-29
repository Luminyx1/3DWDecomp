#pragma once

#include "Player/PlayerActionCondition.hpp"

class PlayerActionConditionRefOrg;

/// Reuses the result another node's PlayerActionConditionRefOrg got this frame.
class PlayerActionConditionRef : public PlayerActionCondition {
public:
    PlayerActionConditionRef(const PlayerActionConditionRefOrg*);

    bool check() override;

private:
    const PlayerActionConditionRefOrg* mRefOrg;  // 0x8
};

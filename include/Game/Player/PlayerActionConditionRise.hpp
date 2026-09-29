#pragma once

#include "Player/PlayerActionCondition.hpp"

struct PlayerProperty;

/// Holds while the player moves upwards.
class PlayerActionConditionRise : public PlayerActionCondition {
public:
    PlayerActionConditionRise(const PlayerProperty*);

    bool check() override;

private:
    const PlayerProperty* mProperty;  // 0x8
};

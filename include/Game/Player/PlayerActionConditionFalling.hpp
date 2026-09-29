#pragma once

#include "Player/PlayerActionCondition.hpp"

struct PlayerProperty;

/// Holds while the player moves along gravity.
class PlayerActionConditionFalling : public PlayerActionCondition {
public:
    PlayerActionConditionFalling(const PlayerProperty* pProperty) : mProperty(pProperty) {}

    bool check() override;

private:
    const PlayerProperty* mProperty;  // 0x8
};

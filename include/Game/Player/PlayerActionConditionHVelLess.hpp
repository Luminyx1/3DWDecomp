#pragma once

#include <basis/seadTypes.h>

#include "Player/PlayerActionCondition.hpp"

struct PlayerProperty;

/// Holds while the player's horizontal speed is below a limit.
class PlayerActionConditionHVelLess : public PlayerActionCondition {
public:
    PlayerActionConditionHVelLess(const PlayerProperty*, f32);

    bool check() override;

private:
    const PlayerProperty* mProperty;  // 0x8
    f32 mMaxSpeedSq;                  // 0x10
};

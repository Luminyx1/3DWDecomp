#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerCollision;
struct PlayerProperty;

/// Holds when the player lands while not moving upwards.
class PlayerActionConditionAirMoveToGroundMove : public PlayerActionCondition {
public:
    PlayerActionConditionAirMoveToGroundMove(const IUsePlayerCollision*, const PlayerProperty*);

    bool check() override;

private:
    const IUsePlayerCollision* mCollision;  // 0x8
    const PlayerProperty* mProperty;  // 0x10
};

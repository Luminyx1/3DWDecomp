#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerCollision;
class IUsePlayerInput;
struct PlayerProperty;

/// Holds when the player lands without tilting the stick.
class PlayerActionConditionAirMoveToLand : public PlayerActionCondition {
public:
    PlayerActionConditionAirMoveToLand(const PlayerProperty*, const IUsePlayerInput*, const IUsePlayerCollision*);

    bool check() override;

private:
    const PlayerProperty* mProperty;  // 0x8
    const IUsePlayerInput* mInput;  // 0x10
    const IUsePlayerCollision* mCollision;  // 0x18
};

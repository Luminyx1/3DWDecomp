#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerCollision;
class IUsePlayerInput;
struct PlayerProperty;

/// Holds when the player lands holding the squat button.
class PlayerActionConditionAirMoveToSquat : public PlayerActionCondition {
public:
    PlayerActionConditionAirMoveToSquat(const IUsePlayerCollision*, const PlayerProperty*, const IUsePlayerInput*);

    bool check() override;

private:
    const IUsePlayerCollision* mCollision;  // 0x8
    const PlayerProperty* mProperty;  // 0x10
    const IUsePlayerInput* mInput;  // 0x18
};

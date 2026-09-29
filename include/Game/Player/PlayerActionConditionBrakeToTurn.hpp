#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerActionEnd;
class IUsePlayerInput;
struct PlayerProperty;

/// Holds when the brake is over and the stick points backwards.
class PlayerActionConditionBrakeToTurn : public PlayerActionCondition {
public:
    PlayerActionConditionBrakeToTurn(const IUsePlayerInput*, const PlayerProperty*, const IUsePlayerActionEnd*);

    bool check() override;

private:
    const IUsePlayerInput* mInput;  // 0x8
    const PlayerProperty* mProperty;  // 0x10
    const IUsePlayerActionEnd* mActionEnd;  // 0x18
};

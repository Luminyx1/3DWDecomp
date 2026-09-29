#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerInput;
struct PlayerProperty;

/// Holds when a jump is input while the stick points backwards.
class PlayerActionConditionBrakeToTurnJump : public PlayerActionCondition {
public:
    PlayerActionConditionBrakeToTurnJump(const IUsePlayerInput*, const PlayerProperty*);

    bool check() override;

private:
    const IUsePlayerInput* mInput;  // 0x8
    const PlayerProperty* mProperty;  // 0x10
};

#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerInput;

/// Holds on the frame the roll button is pressed.
class PlayerActionConditionRollingTrigger : public PlayerActionCondition {
public:
    PlayerActionConditionRollingTrigger(const IUsePlayerInput*);

    bool check() override;

private:
    const IUsePlayerInput* mInput;  // 0x8
};

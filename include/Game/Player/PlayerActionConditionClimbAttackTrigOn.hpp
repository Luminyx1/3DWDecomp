#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerInput;

/// Holds on the frame the climb attack button is pressed.
class PlayerActionConditionClimbAttackTrigOn : public PlayerActionCondition {
public:
    PlayerActionConditionClimbAttackTrigOn(const IUsePlayerInput*);

    bool check() override;

private:
    const IUsePlayerInput* mInput;  // 0x8
};

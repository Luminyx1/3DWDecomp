#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerInput;

/// Holds while the climb attack button stays held since the action started.
class PlayerActionConditionClimbAttackHoldOn : public PlayerActionCondition {
public:
    PlayerActionConditionClimbAttackHoldOn(const IUsePlayerInput*);

    bool check() override;
    void setup() override;

private:
    const IUsePlayerInput* mInput;  // 0x8
    bool mIsHolding = true;         // 0x10
};

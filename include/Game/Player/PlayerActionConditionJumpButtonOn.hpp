#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerInput;

/// Holds while the jump button is held.
class PlayerActionConditionJumpButtonOn : public PlayerActionCondition {
public:
    PlayerActionConditionJumpButtonOn(const IUsePlayerInput* pInput) : mInput(pInput) {}

    bool check() override;

private:
    const IUsePlayerInput* mInput;  // 0x8
};

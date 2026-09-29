#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerInput;

/// Holds on the frame a swim stroke is input.
class PlayerActionConditionSwimPaddleTrigOn : public PlayerActionCondition {
public:
    PlayerActionConditionSwimPaddleTrigOn(const IUsePlayerInput* pInput) : mInput(pInput) {}

    bool check() override;

private:
    const IUsePlayerInput* mInput;  // 0x8
};

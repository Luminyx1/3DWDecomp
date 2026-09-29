#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerInput;

/// Holds while the statue button (tanooki) is held.
class PlayerActionConditionStoneStatueSustainButton : public PlayerActionCondition {
public:
    PlayerActionConditionStoneStatueSustainButton(const IUsePlayerInput*);

    bool check() override;

private:
    const IUsePlayerInput* mInput;  // 0x8
};

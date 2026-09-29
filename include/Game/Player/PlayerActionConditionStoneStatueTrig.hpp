#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerInput;

/// Holds on the frame the statue button (tanooki) is pressed.
class PlayerActionConditionStoneStatueTrig : public PlayerActionCondition {
public:
    PlayerActionConditionStoneStatueTrig(const IUsePlayerInput*);

    bool check() override;

private:
    const IUsePlayerInput* mInput;  // 0x8
};

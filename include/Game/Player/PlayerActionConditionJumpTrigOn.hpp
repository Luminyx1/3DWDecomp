#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerInput;

/// Holds on the frame a jump is input.
class PlayerActionConditionJumpTrigOn : public PlayerActionCondition {
public:
    PlayerActionConditionJumpTrigOn(const IUsePlayerInput*);

    bool check() override;

private:
    const IUsePlayerInput* mInput;  // 0x8
};

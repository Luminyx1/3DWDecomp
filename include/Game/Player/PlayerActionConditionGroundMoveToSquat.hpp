#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerInput;

/// Holds while the squat button is held.
class PlayerActionConditionGroundMoveToSquat : public PlayerActionCondition {
public:
    PlayerActionConditionGroundMoveToSquat(const IUsePlayerInput*);

    bool check() override;

private:
    const IUsePlayerInput* mInput;  // 0x8
};

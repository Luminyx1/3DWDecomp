#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerInput;

/// Holds while the stick is tilted.
class PlayerActionConditionStickOn : public PlayerActionCondition {
public:
    PlayerActionConditionStickOn(const IUsePlayerInput*);

    bool check() override;

private:
    const IUsePlayerInput* mInput;  // 0x8
};

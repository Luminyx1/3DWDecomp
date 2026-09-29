#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerSubAction;

/// Holds while a sub action (throw, tail attack, ...) runs.
class PlayerActionConditionSubActionRunning : public PlayerActionCondition {
public:
    PlayerActionConditionSubActionRunning(const IUsePlayerSubAction*);

    bool check() override;

private:
    const IUsePlayerSubAction* mSubAction;  // 0x8
};

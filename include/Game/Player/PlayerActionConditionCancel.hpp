#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerActionCancelable;

/// Holds while the current action can be cancelled.
class PlayerActionConditionCancel : public PlayerActionCondition {
public:
    PlayerActionConditionCancel(const IUsePlayerActionCancelable*);

    bool check() override;

private:
    const IUsePlayerActionCancelable* mCancelable;  // 0x8
};

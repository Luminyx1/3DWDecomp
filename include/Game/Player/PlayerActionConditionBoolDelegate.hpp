#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerBoolDelegate;

/// Asks a callback.
class PlayerActionConditionBoolDelegate : public PlayerActionCondition {
public:
    PlayerActionConditionBoolDelegate(IUsePlayerBoolDelegate*);

    bool check() override;

private:
    IUsePlayerBoolDelegate* mDelegate;  // 0x8
};

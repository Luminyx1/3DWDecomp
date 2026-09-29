#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerActionInhibitor;

/// Holds while an action is blocked.
class PlayerActionConditionIsInhibit : public PlayerActionCondition {
public:
    PlayerActionConditionIsInhibit(const IUsePlayerActionInhibitor*);

    bool check() override;

private:
    const IUsePlayerActionInhibitor* mInhibitor;  // 0x8
};

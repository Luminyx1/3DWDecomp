#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerActionEnd;

/// Holds once the current action has finished.
class PlayerActionConditionCheckEnd : public PlayerActionCondition {
public:
    PlayerActionConditionCheckEnd(const IUsePlayerActionEnd*);

    bool check() override;

private:
    const IUsePlayerActionEnd* mActionEnd;  // 0x8
};

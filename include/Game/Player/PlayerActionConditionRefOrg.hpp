#pragma once

#include "Player/PlayerActionCondition.hpp"

/// Checks a condition and keeps the result so PlayerActionConditionRef can reuse it.
class PlayerActionConditionRefOrg : public PlayerActionCondition {
public:
    PlayerActionConditionRefOrg(PlayerActionCondition*);

    bool check() override;
    void setup() override;

    bool getResult() const { return mResult; }

private:
    PlayerActionCondition* mCondition;  // 0x8
    bool mResult = false;               // 0x10
};

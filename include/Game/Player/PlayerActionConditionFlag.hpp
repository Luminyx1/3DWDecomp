#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerFlag;

/// Holds when a flag has the expected value.
class PlayerActionConditionFlag : public PlayerActionCondition {
public:
    PlayerActionConditionFlag(const IUsePlayerFlag*, bool);

    bool check() override;

private:
    const IUsePlayerFlag* mFlag;  // 0x8
    bool mIsOn;  // 0x10
};

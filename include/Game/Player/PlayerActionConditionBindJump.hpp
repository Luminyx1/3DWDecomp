#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerBindEndParamGetter;

/// Holds when the last bind ended with a launch (the binder left an end parameter).
class PlayerActionConditionBindJump : public PlayerActionCondition {
public:
    PlayerActionConditionBindJump(const IUsePlayerBindEndParamGetter*);

    bool check() override;

private:
    const IUsePlayerBindEndParamGetter* mBindEndParamGetter;  // 0x8
};

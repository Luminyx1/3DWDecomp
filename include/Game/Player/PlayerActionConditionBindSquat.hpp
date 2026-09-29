#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerBindEndParamGetter;

/// Holds when the last bind ended with the player squatting.
class PlayerActionConditionBindSquat : public PlayerActionCondition {
public:
    PlayerActionConditionBindSquat(const IUsePlayerBindEndParamGetter*);

    bool check() override;

private:
    const IUsePlayerBindEndParamGetter* mBindEndParamGetter;  // 0x8
};

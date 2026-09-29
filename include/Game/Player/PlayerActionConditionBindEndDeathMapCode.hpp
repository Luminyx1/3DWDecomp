#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerBindEndParamGetter;

/// Holds when the last bind ended on ground that kills the player.
class PlayerActionConditionBindEndDeathMapCode : public PlayerActionCondition {
public:
    PlayerActionConditionBindEndDeathMapCode(const IUsePlayerBindEndParamGetter*);

    bool check() override;

private:
    const IUsePlayerBindEndParamGetter* mBindEndParamGetter;  // 0x8
};

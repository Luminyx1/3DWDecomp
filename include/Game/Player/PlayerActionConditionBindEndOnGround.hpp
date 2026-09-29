#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerBindEndParamGetter;

/// Holds when the last bind ended with the player on the ground.
class PlayerActionConditionBindEndOnGround : public PlayerActionCondition {
public:
    PlayerActionConditionBindEndOnGround(const IUsePlayerBindEndParamGetter*);

    bool check() override;

private:
    const IUsePlayerBindEndParamGetter* mBindEndParamGetter;  // 0x8
};

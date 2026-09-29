#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerBindEndParamGetter;

/// Holds when the object that bound the player forbids wall actions afterwards.
class PlayerActionConditionWallInhibitAfterBind : public PlayerActionCondition {
public:
    PlayerActionConditionWallInhibitAfterBind(const IUsePlayerBindEndParamGetter*);

    bool check() override;

private:
    const IUsePlayerBindEndParamGetter* mBindEndParamGetter;  // 0x8
};

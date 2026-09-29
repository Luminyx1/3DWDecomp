#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerRaccoonDogFallTask;

/// Holds during the first tanooki slow fall after leaving the ground.
class PlayerActionConditionRaccoonDogFall1st : public PlayerActionCondition {
public:
    PlayerActionConditionRaccoonDogFall1st(const IUsePlayerRaccoonDogFallTask*);

    bool check() override;

private:
    const IUsePlayerRaccoonDogFallTask* mFallTask;  // 0x8
};

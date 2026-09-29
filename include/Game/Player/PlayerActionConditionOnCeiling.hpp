#pragma once

#include <basis/seadTypes.h>

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerCollision;

/// Holds while the player's head touches the ceiling.
class PlayerActionConditionOnCeiling : public PlayerActionCondition {
public:
    PlayerActionConditionOnCeiling(const IUsePlayerCollision* pCollision) : mCollision(pCollision) {}

    bool check() override;
    void setup() override;

private:
    const IUsePlayerCollision* mCollision;  // 0x8
    s32 mCheckFrame = 4;                    // 0x10
};

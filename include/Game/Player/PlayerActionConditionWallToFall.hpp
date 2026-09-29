#pragma once

#include <basis/seadTypes.h>

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerCollision;
class IUsePlayerInput;
class PlayerConstParam;

/// Holds when the player sliding down a wall lets go: the wall is gone, or the stick pushes away from it for a while.
class PlayerActionConditionWallToFall : public PlayerActionCondition {
public:
    PlayerActionConditionWallToFall(const IUsePlayerInput*, const IUsePlayerCollision*, const PlayerConstParam*);

    bool check() override;
    void setup() override;

private:
    const IUsePlayerInput* mInput;          // 0x8
    const IUsePlayerCollision* mCollision;  // 0x10
    const PlayerConstParam* mConstParam;    // 0x18
    u32 mApartFrame = 0;                    // 0x20
    u32 mNoWallFrame = 0;                   // 0x24
};

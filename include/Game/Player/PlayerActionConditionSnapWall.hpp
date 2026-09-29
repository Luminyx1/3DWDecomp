#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerSnapWallInfo;

/// Holds while the player is snapped to a wall.
class PlayerActionConditionSnapWall : public PlayerActionCondition {
public:
    PlayerActionConditionSnapWall(const IUsePlayerSnapWallInfo*);

    bool check() override;

private:
    const IUsePlayerSnapWallInfo* mSnapWallInfo;  // 0x8
};

#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerCollision;
struct PlayerProperty;

/// Holds while the player walks into a wall that faces them.
class PlayerActionConditionFrontWallForGroundWall : public PlayerActionCondition {
public:
    PlayerActionConditionFrontWallForGroundWall(const PlayerProperty*, const IUsePlayerCollision*);

    bool check() override;

private:
    const PlayerProperty* mProperty;        // 0x8
    const IUsePlayerCollision* mCollision;  // 0x10
};

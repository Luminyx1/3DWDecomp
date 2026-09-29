#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerCollision;
struct PlayerProperty;

/// Holds while the player moves down the slope they stand on.
class PlayerActionConditionSlideVel : public PlayerActionCondition {
public:
    PlayerActionConditionSlideVel(const PlayerProperty*, const IUsePlayerCollision*);

    bool check() override;

private:
    const PlayerProperty* mProperty;        // 0x8
    const IUsePlayerCollision* mCollision;  // 0x10
};

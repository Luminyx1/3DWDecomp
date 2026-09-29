#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerWallClimbInfo;

/// Holds once the cat suit has climbed a wall for too long.
class PlayerActionConditionUnableWallClimb : public PlayerActionCondition {
public:
    PlayerActionConditionUnableWallClimb(const IUsePlayerWallClimbInfo*);

    bool check() override;

private:
    const IUsePlayerWallClimbInfo* mWallClimbInfo;  // 0x8
};

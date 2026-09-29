#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerWallJumpInfo;
struct PlayerProperty;

/// Holds when the player is below the last wall jump position; always true in this version.
class PlayerActionConditionUnderWallJumpPos : public PlayerActionCondition {
public:
    PlayerActionConditionUnderWallJumpPos(const PlayerProperty*, const IUsePlayerWallJumpInfo*);

    bool check() override;

private:
    const PlayerProperty* mProperty;  // 0x8
    const IUsePlayerWallJumpInfo* mWallJumpInfo;  // 0x10
};

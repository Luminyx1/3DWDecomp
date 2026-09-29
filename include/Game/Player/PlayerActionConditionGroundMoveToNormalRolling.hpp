#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerInput;
class PlayerConstParam;
struct PlayerProperty;

/// Holds when squat is pressed while running fast enough to roll.
class PlayerActionConditionGroundMoveToNormalRolling : public PlayerActionCondition {
public:
    PlayerActionConditionGroundMoveToNormalRolling(const PlayerProperty*, const IUsePlayerInput*, const PlayerConstParam*);

    bool check() override;

private:
    const PlayerProperty* mProperty;  // 0x8
    const IUsePlayerInput* mInput;  // 0x10
    const PlayerConstParam* mConstParam;  // 0x18
};

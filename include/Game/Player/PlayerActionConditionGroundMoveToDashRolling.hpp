#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerDashChecker;
class IUsePlayerInput;
class PlayerConstParam;
struct PlayerProperty;

/// Holds when squat is pressed while dashing faster than a normal run.
class PlayerActionConditionGroundMoveToDashRolling : public PlayerActionCondition {
public:
    PlayerActionConditionGroundMoveToDashRolling(const PlayerProperty*, const IUsePlayerDashChecker*, const IUsePlayerInput*, const PlayerConstParam*);

    bool check() override;

private:
    const PlayerProperty* mProperty;  // 0x8
    const IUsePlayerDashChecker* mDashChecker;  // 0x10
    const IUsePlayerInput* mInput;  // 0x18
    const PlayerConstParam* mConstParam;  // 0x20
};

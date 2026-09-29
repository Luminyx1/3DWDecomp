#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerCeilingCheck;
class IUsePlayerCollision;
class IUsePlayerInput;
struct PlayerProperty;

/// Holds when a roll ends on the ground with squat held or no room to stand.
class PlayerActionConditionRollingAttackToSquat : public PlayerActionCondition {
public:
    PlayerActionConditionRollingAttackToSquat(const IUsePlayerCollision*, const PlayerProperty*, const IUsePlayerInput*, const IUsePlayerCeilingCheck*);

    bool check() override;

private:
    const IUsePlayerCollision* mCollision;  // 0x8
    const PlayerProperty* mProperty;  // 0x10
    const IUsePlayerInput* mInput;  // 0x18
    const IUsePlayerCeilingCheck* mCeilingCheck;  // 0x20
};

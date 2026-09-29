#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerCollision;
class IUsePlayerDamageInvalidCheck;

/// Holds while the player touches ground with a map code (e.g. "Poison", "Needle").
class PlayerActionConditionFloorCode : public PlayerActionCondition {
public:
    PlayerActionConditionFloorCode(const IUsePlayerCollision*, const char*, bool, const IUsePlayerDamageInvalidCheck*);

    bool check() override;

private:
    const IUsePlayerCollision* mCollision;                    // 0x8
    const char* mCode;                                        // 0x10
    bool mIsOnFloorOnly;                                      // 0x18
    const IUsePlayerDamageInvalidCheck* mDamageInvalidCheck;  // 0x20
};

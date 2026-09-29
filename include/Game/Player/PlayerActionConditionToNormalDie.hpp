#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerIsDamageEnd;
class IUsePlayerLifeControl;

/// Holds when the player is dying and any damage reaction is over.
class PlayerActionConditionToNormalDie : public PlayerActionCondition {
public:
    PlayerActionConditionToNormalDie(const IUsePlayerLifeControl*, const IUsePlayerIsDamageEnd*);

    bool check() override;

private:
    const IUsePlayerIsDamageEnd* mDamageEnd;  // 0x8
    const IUsePlayerLifeControl* mLifeControl;  // 0x10
};

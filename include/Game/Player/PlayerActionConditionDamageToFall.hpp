#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerIsDamageEnd;

/// Holds once the damage reaction is over.
class PlayerActionConditionDamageToFall : public PlayerActionCondition {
public:
    PlayerActionConditionDamageToFall(const IUsePlayerIsDamageEnd*);

    bool check() override;

private:
    const IUsePlayerIsDamageEnd* mDamageEnd;  // 0x8
};

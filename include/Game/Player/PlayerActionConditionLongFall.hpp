#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerLongFallCheck;

/// Holds once the player has been falling for long.
class PlayerActionConditionLongFall : public PlayerActionCondition {
public:
    PlayerActionConditionLongFall(const IUsePlayerLongFallCheck*);

    bool check() override;

private:
    const IUsePlayerLongFallCheck* mLongFallCheck;  // 0x8
};

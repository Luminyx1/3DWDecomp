#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerCeilingCheck;

/// Holds when there is room to stand up.
class PlayerActionConditionCeilingCheck : public PlayerActionCondition {
public:
    PlayerActionConditionCeilingCheck(const IUsePlayerCeilingCheck*);

    bool check() override;

private:
    const IUsePlayerCeilingCheck* mCeilingCheck;  // 0x8
};

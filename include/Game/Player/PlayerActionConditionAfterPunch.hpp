#pragma once

#include <basis/seadTypes.h>

#include "Player/PlayerActionCondition.hpp"

class PlayerCounterAfterPunch;

/// Holds once enough frames passed since the last punch.
class PlayerActionConditionAfterPunch : public PlayerActionCondition {
public:
    PlayerActionConditionAfterPunch(const PlayerCounterAfterPunch*, u32);

    bool check() override;

private:
    const PlayerCounterAfterPunch* mCounter;  // 0x8
    u32 mFrame;  // 0x10
};

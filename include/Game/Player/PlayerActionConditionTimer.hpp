#pragma once

#include <basis/seadTypes.h>

#include "Player/PlayerActionCondition.hpp"

/// Holds once another condition (or nothing, if none) has held for a number of frames in a row.
class PlayerActionConditionTimer : public PlayerActionCondition {
public:
    PlayerActionConditionTimer(u32, PlayerActionCondition*);

    bool check() override;
    void setup() override;

private:
    PlayerActionCondition* mCondition;  // 0x8
    u32 mFrame;                         // 0x10
    u32 mCounter = 0;                   // 0x14
};

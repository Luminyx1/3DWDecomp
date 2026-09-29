#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerSizeTrigger;

/// Holds on the frame the player grows big.
class PlayerActionConditionBigTrig : public PlayerActionCondition {
public:
    PlayerActionConditionBigTrig(const IUsePlayerSizeTrigger*);

    bool check() override;

private:
    const IUsePlayerSizeTrigger* mSizeTrigger;  // 0x8
};

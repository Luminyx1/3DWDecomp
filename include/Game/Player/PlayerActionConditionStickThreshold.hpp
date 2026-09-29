#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerInput;
struct PlayerActionArg;

/// Holds while the stick is tilted far.
class PlayerActionConditionStickThreshold : public PlayerActionCondition {
public:
    PlayerActionConditionStickThreshold(const PlayerActionArg*, const IUsePlayerInput*);

    bool check() override;

private:
    const PlayerActionArg* mArg;    // 0x8
    const IUsePlayerInput* mInput;  // 0x10
};

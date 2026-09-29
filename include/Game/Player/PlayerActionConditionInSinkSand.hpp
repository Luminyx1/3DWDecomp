#pragma once

#include "Player/PlayerActionCondition.hpp"

class SinkSandControl;

/// Holds while the player sinks into quicksand.
class PlayerActionConditionInSinkSand : public PlayerActionCondition {
public:
    PlayerActionConditionInSinkSand(const SinkSandControl*);

    bool check() override;

private:
    const SinkSandControl* mSinkSandControl;  // 0x8
};

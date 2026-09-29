#pragma once

#include "Player/PlayerActionCondition.hpp"

class PlayerClimbAirAttackInhibitor;

/// Holds while the climb air attack is blocked.
class PlayerActionConditionClimbAirAttackInhibit : public PlayerActionCondition {
public:
    PlayerActionConditionClimbAirAttackInhibit(PlayerClimbAirAttackInhibitor*);

    bool check() override;

private:
    PlayerClimbAirAttackInhibitor* mInhibitor;  // 0x8
};

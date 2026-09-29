#pragma once

#include "Player/PlayerActionCondition.hpp"
#include "Player/Normal/PlayerTrigger.hpp"

/// Holds on the frame a sensor or collision event happens.
class PlayerActionConditionTrigger : public PlayerActionCondition {
public:
    PlayerActionConditionTrigger(const PlayerTrigger*, PlayerTrigger::ESensorTrigger);
    PlayerActionConditionTrigger(const PlayerTrigger*, PlayerTrigger::ECollisionTrigger);

    bool check() override;

private:
    const PlayerTrigger* mTrigger;                        // 0x8
    bool mIsSensor;                                       // 0x10
    PlayerTrigger::ESensorTrigger mSensorTrigger;         // 0x14
    PlayerTrigger::ECollisionTrigger mCollisionTrigger;  // 0x18
};

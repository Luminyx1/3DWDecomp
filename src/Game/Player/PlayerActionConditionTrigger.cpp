#include "Player/PlayerActionConditionTrigger.hpp"

/**
 * Holds on the frame a sensor event happens.
 * @param pTrigger the player's triggers
 * @param sensorTrigger sensor event to wait for
 */
PlayerActionConditionTrigger::PlayerActionConditionTrigger(const PlayerTrigger* pTrigger,
                                                           PlayerTrigger::ESensorTrigger sensorTrigger)
    : mTrigger(pTrigger), mIsSensor(true), mSensorTrigger(sensorTrigger),
      mCollisionTrigger(PlayerTrigger::cCollisionTriggerNum) {}

/**
 * Holds on the frame a collision event happens.
 * @param pTrigger the player's triggers
 * @param collisionTrigger collision event to wait for
 */
PlayerActionConditionTrigger::PlayerActionConditionTrigger(const PlayerTrigger* pTrigger,
                                                           PlayerTrigger::ECollisionTrigger collisionTrigger)
    : mTrigger(pTrigger), mIsSensor(false), mSensorTrigger(PlayerTrigger::cSensorTriggerNum),
      mCollisionTrigger(collisionTrigger) {}

/**
 * @return whether the event happened this frame
 */
bool PlayerActionConditionTrigger::check() {
    if (mIsSensor) {
        return mTrigger->isOn(mSensorTrigger);
    }

    return mTrigger->isOn(mCollisionTrigger);
}

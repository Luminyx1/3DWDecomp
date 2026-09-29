#include "Player/PlayerActionConditionRise.hpp"
#include "Player/Normal/PlayerProperty.hpp"

/**
 * Holds while the player moves upwards.
 * @param pProperty the player's physical state
 */
PlayerActionConditionRise::PlayerActionConditionRise(const PlayerProperty* pProperty) : mProperty(pProperty) {}

/**
 * @return whether the player is rising
 */
bool PlayerActionConditionRise::check() {
    return mProperty->mUpDir.dot(mProperty->mVelocity) > 0.0f;
}

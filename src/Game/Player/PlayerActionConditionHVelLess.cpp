#include "Player/PlayerActionConditionHVelLess.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Player/Normal/PlayerProperty.hpp"

/**
 * Holds while the player's horizontal speed is below a limit.
 * @param pProperty the player's physical state
 * @param maxSpeed speed limit
 */
PlayerActionConditionHVelLess::PlayerActionConditionHVelLess(const PlayerProperty* pProperty, f32 maxSpeed)
    : mProperty(pProperty), mMaxSpeedSq(maxSpeed * maxSpeed) {}

/**
 * @return whether the velocity across the up direction is slower than the limit
 */
bool PlayerActionConditionHVelLess::check() {
    sead::Vector3f hVel;
    al::verticalizeVec(&hVel, mProperty->getUpDir(), mProperty->getVelocity());
    return hVel.squaredLength() < mMaxSpeedSq;
}

#include "Player/PlayerActionConditionFalling.hpp"
#include "Player/Normal/PlayerProperty.hpp"

/**
 * @return whether the player is falling
 */
bool PlayerActionConditionFalling::check() {
    return mProperty->mGravity.dot(mProperty->mVelocity) > 0.0f;
}

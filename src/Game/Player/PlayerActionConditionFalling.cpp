#include "Player/PlayerActionConditionFalling.hpp"
#include "Player/Normal/PlayerProperty.hpp"

/**
 * @return whether the player is falling
 */
bool PlayerActionConditionFalling::check() {
    return mProperty->getGravity().dot(mProperty->getVelocity()) > 0.0f;
}

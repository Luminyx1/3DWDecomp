#include "Player/PlayerActionConditionOnCeiling.hpp"
#include "Player/IUsePlayerCollision.hpp"

/**
 * @return whether the player touches the ceiling
 */
bool PlayerActionConditionOnCeiling::check() {
    return mCollision->isOnCeiling();
}

/**
 * Resets the frame count.
 */
void PlayerActionConditionOnCeiling::setup() {
    mCheckFrame = 4;
}

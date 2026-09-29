#include "Player/PlayerActionConditionOnFloor.hpp"
#include "Player/IUsePlayerCollision.hpp"

/**
 * @return whether the player is on the floor
 */
bool PlayerActionConditionOnFloor::check() {
    return mCollision->isOnFloor();
}

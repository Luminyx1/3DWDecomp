#include "Player/PlayerActionConditionWallToGroundMove.hpp"
#include "Player/IUsePlayerCollision.hpp"

/**
 * Holds once the player clinging to a wall touches the floor.
 * @param pCollision the player's collision
 */
PlayerActionConditionWallToGroundMove::PlayerActionConditionWallToGroundMove(const IUsePlayerCollision* pCollision) : mCollision(pCollision) {}

/**
 * @return whether the player is on the floor
 */
bool PlayerActionConditionWallToGroundMove::check() {
    return mCollision->isOnFloor();
}

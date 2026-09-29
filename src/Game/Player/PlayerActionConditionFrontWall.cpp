#include "Player/PlayerActionConditionFrontWall.hpp"
#include "Player/IUsePlayerCollision.hpp"

/**
 * Holds while the player touches a wall in front.
 * @param pCollision the player's collision
 */
PlayerActionConditionFrontWall::PlayerActionConditionFrontWall(const IUsePlayerCollision* pCollision) : mCollision(pCollision) {}

/**
 * @return whether there is a wall in front
 */
bool PlayerActionConditionFrontWall::check() {
    return mCollision->isOnFrontWall();
}

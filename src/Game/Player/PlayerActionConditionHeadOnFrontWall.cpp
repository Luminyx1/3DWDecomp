#include "Player/PlayerActionConditionHeadOnFrontWall.hpp"
#include "Player/IUsePlayerCollision.hpp"

/**
 * Holds while the player's head touches a wall in front.
 * @param pCollision the player's collision
 */
PlayerActionConditionHeadOnFrontWall::PlayerActionConditionHeadOnFrontWall(const IUsePlayerCollision* pCollision) : mCollision(pCollision) {}

/**
 * @return whether the head touches a wall in front
 */
bool PlayerActionConditionHeadOnFrontWall::check() {
    return mCollision->isHeadOnFrontWall();
}

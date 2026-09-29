#include "Player/PlayerActionConditionFrontWallForGroundWall.hpp"
#include "Player/IUsePlayerCollision.hpp"
#include "Player/Normal/PlayerProperty.hpp"

/**
 * Holds while the player walks into a wall that faces them.
 * @param pProperty the player's physical state
 * @param pCollision the player's collision
 */
PlayerActionConditionFrontWallForGroundWall::PlayerActionConditionFrontWallForGroundWall(
    const PlayerProperty* pProperty, const IUsePlayerCollision* pCollision)
    : mProperty(pProperty), mCollision(pCollision) {}

/**
 * @return whether the wall in front is at most about 78 degrees off facing the player
 */
bool PlayerActionConditionFrontWallForGroundWall::check() {
    if (!mCollision->isOnFrontWall()) {
        return false;
    }
    IUsePlayerCollision::Info info = {};
    mCollision->getFrontWallInfo(&info);
    return mProperty->mFront.dot(info.mNormal) <= -0.2f;
}

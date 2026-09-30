#include "Player/PlayerActionConditionFrontWallNoAction.hpp"
#include "Player/IUsePlayerCollision.hpp"
#include "Project/Base/StringUtil.hpp"

/**
 * Holds while the wall in front forbids wall actions.
 * @param pCollision the player's collision
 */
PlayerActionConditionFrontWallNoAction::PlayerActionConditionFrontWallNoAction(const IUsePlayerCollision* pCollision)
    : mCollision(pCollision) {}

/**
 * @return whether there is a "NoAction" wall in front
 */
bool PlayerActionConditionFrontWallNoAction::check() {
    if (!mCollision->isOnFrontWall()) {
        return false;
    }

    IUsePlayerCollision::Info info = {};
    mCollision->getFrontWallInfo(&info);
    return al::isEqualString(info.mWallCode, "NoAction");
}

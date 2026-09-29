#include "Player/PlayerActionConditionSnapWall.hpp"
#include "Player/IUsePlayerSnapWallInfo.hpp"

/**
 * Holds while the player is snapped to a wall.
 * @param pSnapWallInfo snapped wall info
 */
PlayerActionConditionSnapWall::PlayerActionConditionSnapWall(const IUsePlayerSnapWallInfo* pSnapWallInfo) : mSnapWallInfo(pSnapWallInfo) {}

/**
 * @return whether there is a wall to snap to
 */
bool PlayerActionConditionSnapWall::check() {
    return mSnapWallInfo->isSnapWallExist();
}

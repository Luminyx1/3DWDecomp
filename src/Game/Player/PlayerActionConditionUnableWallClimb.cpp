#include "Player/PlayerActionConditionUnableWallClimb.hpp"
#include "Player/IUsePlayerWallClimbInfo.hpp"

/**
 * Holds once the cat suit has climbed a wall for too long.
 * @param pWallClimbInfo wall climb time
 */
PlayerActionConditionUnableWallClimb::PlayerActionConditionUnableWallClimb(const IUsePlayerWallClimbInfo* pWallClimbInfo) : mWallClimbInfo(pWallClimbInfo) {}

/**
 * @return whether the climb time is used up
 */
bool PlayerActionConditionUnableWallClimb::check() {
    return mWallClimbInfo->isWallClimbCountOver();
}

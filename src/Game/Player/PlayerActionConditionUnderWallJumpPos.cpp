#include "Player/PlayerActionConditionUnderWallJumpPos.hpp"
#include "Player/Normal/PlayerProperty.hpp"

/**
 * Holds when the player is below the last wall jump position; always true in this version.
 * @param pProperty the player's physical state
 * @param pWallJumpInfo last wall jump
 */
PlayerActionConditionUnderWallJumpPos::PlayerActionConditionUnderWallJumpPos(const PlayerProperty* pProperty, const IUsePlayerWallJumpInfo* pWallJumpInfo) : mProperty(pProperty), mWallJumpInfo(pWallJumpInfo) {}

/**
 * @return true
 */
bool PlayerActionConditionUnderWallJumpPos::check() {
    return true;
}

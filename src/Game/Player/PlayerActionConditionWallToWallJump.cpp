#include "Player/PlayerActionConditionWallToWallJump.hpp"
#include "Player/IUsePlayerCollision.hpp"
#include "Player/IUsePlayerInput.hpp"

/**
 * Holds when a jump is input while the player's back is to a wall.
 * @param pInput the player's input
 * @param pCollision the player's collision
 */
PlayerActionConditionWallToWallJump::PlayerActionConditionWallToWallJump(const IUsePlayerInput* pInput, const IUsePlayerCollision* pCollision)
    : mInput(pInput), mCollision(pCollision) {}

/**
 * @return whether to wall jump
 */
bool PlayerActionConditionWallToWallJump::check() {
    return mInput->isJumpTrigOn() && mCollision->isOnBackWall();
}

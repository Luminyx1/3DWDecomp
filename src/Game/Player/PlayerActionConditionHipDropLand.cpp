#include "Player/PlayerActionConditionHipDropLand.hpp"
#include "Player/IUsePlayerAnimator.hpp"
#include "Player/IUsePlayerCollision.hpp"

/**
 * Holds when the ground pound has landed.
 * @param pAnimator the player's animator
 * @param pCollision the player's collision
 */
PlayerActionConditionHipDropLand::PlayerActionConditionHipDropLand(const IUsePlayerAnimator* pAnimator,
                                                                   const IUsePlayerCollision* pCollision)
    : mAnimator(pAnimator), mCollision(pCollision) {}

/**
 * @return whether a (giga) ground pound landing plays past its first frame on the floor
 */
bool PlayerActionConditionHipDropLand::check() {
    return (mAnimator->isAnim("HipDropLand") || mAnimator->isAnim("GigaHipDropLand")) &&
           mAnimator->getAnimFrame() >= 1.0f && mCollision->isOnFloor();
}

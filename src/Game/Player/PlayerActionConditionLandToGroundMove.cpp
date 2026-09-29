#include "Player/PlayerActionConditionLandToGroundMove.hpp"
#include "Player/IUsePlayerActionCancelable.hpp"
#include "Player/IUsePlayerCollision.hpp"
#include "Player/IUsePlayerInput.hpp"

/**
 * Holds when the landing can be cancelled into walking.
 * @param pInput the player's input
 * @param pCollision the player's collision
 * @param pCancelable action to ask
 */
PlayerActionConditionLandToGroundMove::PlayerActionConditionLandToGroundMove(const IUsePlayerInput* pInput, const IUsePlayerCollision* pCollision, const IUsePlayerActionCancelable* pCancelable)
    : mInput(pInput), mCollision(pCollision), mCancelable(pCancelable) {}

/**
 * @return whether to start walking
 */
bool PlayerActionConditionLandToGroundMove::check() {
    return mCancelable->isPossibleToCancel() && mInput->isStickOn() && mCollision->isOnFloor();
}

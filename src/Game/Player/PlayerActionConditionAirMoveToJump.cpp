#include "Player/PlayerActionConditionAirMoveToJump.hpp"
#include "Player/IUsePlayerCollision.hpp"
#include "Player/IUsePlayerInput.hpp"
#include "Player/PlayerActionFunc.hpp"
#include "Player/Normal/PlayerProperty.hpp"

/**
 * Holds when a jump is input as the player lands.
 * @param pCollision the player's collision
 * @param pProperty the player's physical state
 * @param pInput the player's input
 */
PlayerActionConditionAirMoveToJump::PlayerActionConditionAirMoveToJump(const IUsePlayerCollision* pCollision, const PlayerProperty* pProperty, const IUsePlayerInput* pInput)
    : mCollision(pCollision), mProperty(pProperty), mInput(pInput) {}

/**
 * @return whether to jump on landing
 */
bool PlayerActionConditionAirMoveToJump::check() {
    return mInput->isJumpTrigOn() && !PlayerActionFunc::isUpperVelocity(mProperty) && mCollision->isOnFloor();
}

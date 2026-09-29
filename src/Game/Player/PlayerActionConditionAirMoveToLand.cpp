#include "Player/PlayerActionConditionAirMoveToLand.hpp"
#include "Player/IUsePlayerCollision.hpp"
#include "Player/IUsePlayerInput.hpp"
#include "Player/PlayerActionFunc.hpp"
#include "Player/Normal/PlayerProperty.hpp"

/**
 * Holds when the player lands without tilting the stick.
 * @param pProperty the player's physical state
 * @param pInput the player's input
 * @param pCollision the player's collision
 */
PlayerActionConditionAirMoveToLand::PlayerActionConditionAirMoveToLand(const PlayerProperty* pProperty, const IUsePlayerInput* pInput, const IUsePlayerCollision* pCollision)
    : mProperty(pProperty), mInput(pInput), mCollision(pCollision) {}

/**
 * @return whether to play the landing
 */
bool PlayerActionConditionAirMoveToLand::check() {
    return mCollision->isOnFloor() && !PlayerActionFunc::isUpperVelocity(mProperty) && !mInput->isStickOn();
}

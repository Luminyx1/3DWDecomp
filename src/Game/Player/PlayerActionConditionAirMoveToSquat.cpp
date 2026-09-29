#include "Player/PlayerActionConditionAirMoveToSquat.hpp"
#include "Player/IUsePlayerCollision.hpp"
#include "Player/IUsePlayerInput.hpp"
#include "Player/PlayerActionFunc.hpp"
#include "Player/Normal/PlayerProperty.hpp"

/**
 * Holds when the player lands holding the squat button.
 * @param pCollision the player's collision
 * @param pProperty the player's physical state
 * @param pInput the player's input
 */
PlayerActionConditionAirMoveToSquat::PlayerActionConditionAirMoveToSquat(const IUsePlayerCollision* pCollision, const PlayerProperty* pProperty, const IUsePlayerInput* pInput)
    : mCollision(pCollision), mProperty(pProperty), mInput(pInput) {}

/**
 * @return whether to squat on landing
 */
bool PlayerActionConditionAirMoveToSquat::check() {
    return mInput->isSquatButtonOn() && !PlayerActionFunc::isUpperVelocity(mProperty) && mCollision->isOnFloor();
}

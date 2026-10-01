#include "Player/PlayerActionConditionGroundMoveToDashRolling.hpp"
#include "Player/IUsePlayerDashChecker.hpp"
#include "Player/IUsePlayerInput.hpp"
#include "Player/Normal/PlayerConstParam.hpp"
#include "Player/Normal/PlayerProperty.hpp"

/**
 * Holds when squat is pressed while dashing faster than a normal run.
 * @param pProperty the player's physical state
 * @param pDashChecker dash state
 * @param pInput the player's input
 * @param pConstParam the player's tuning values
 */
PlayerActionConditionGroundMoveToDashRolling::PlayerActionConditionGroundMoveToDashRolling(const PlayerProperty* pProperty, const IUsePlayerDashChecker* pDashChecker, const IUsePlayerInput* pInput, const PlayerConstParam* pConstParam)
    : mProperty(pProperty), mDashChecker(pDashChecker), mInput(pInput), mConstParam(pConstParam) {}

/**
 * @return whether to start a dash roll
 */
bool PlayerActionConditionGroundMoveToDashRolling::check() {
    return mInput->isSquatTrigOn() && mDashChecker->isDashing() &&
           mProperty->getVelocity().length() > mConstParam->getNormalMaxSpeed();
}

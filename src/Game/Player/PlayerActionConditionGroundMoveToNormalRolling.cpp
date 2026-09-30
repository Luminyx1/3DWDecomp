#include "Player/PlayerActionConditionGroundMoveToNormalRolling.hpp"
#include "Player/IUsePlayerInput.hpp"
#include "Player/Normal/PlayerConstParam.hpp"
#include "Player/Normal/PlayerProperty.hpp"

/**
 * Holds when squat is pressed while running fast enough to roll.
 * @param pProperty the player's physical state
 * @param pInput the player's input
 * @param pConstParam the player's tuning values
 */
PlayerActionConditionGroundMoveToNormalRolling::PlayerActionConditionGroundMoveToNormalRolling(const PlayerProperty* pProperty, const IUsePlayerInput* pInput, const PlayerConstParam* pConstParam)
    : mProperty(pProperty), mInput(pInput), mConstParam(pConstParam) {}

/**
 * @return whether to start a roll
 */
bool PlayerActionConditionGroundMoveToNormalRolling::check() {
    if (!mInput->isSquatTrigOn()) {
        return false;
    }

    return mProperty->mVelocity.length() > mConstParam->getSquatShiftSpeedRate() * mConstParam->getNormalMaxSpeed();
}

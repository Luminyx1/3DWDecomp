#include "Player/PlayerActionConditionRollingAttackToSquat.hpp"
#include "Player/IUsePlayerCeilingCheck.hpp"
#include "Player/IUsePlayerCollision.hpp"
#include "Player/IUsePlayerInput.hpp"
#include "Player/PlayerActionFunc.hpp"
#include "Player/Normal/PlayerProperty.hpp"

/**
 * Holds when a roll ends on the ground with squat held or no room to stand.
 * @param pCollision the player's collision
 * @param pProperty the player's physical state
 * @param pInput the player's input
 * @param pCeilingCheck ceiling check
 */
PlayerActionConditionRollingAttackToSquat::PlayerActionConditionRollingAttackToSquat(const IUsePlayerCollision* pCollision, const PlayerProperty* pProperty, const IUsePlayerInput* pInput, const IUsePlayerCeilingCheck* pCeilingCheck)
    : mCollision(pCollision), mProperty(pProperty), mInput(pInput), mCeilingCheck(pCeilingCheck) {}

/**
 * @return whether to squat after the roll
 */
bool PlayerActionConditionRollingAttackToSquat::check() {
    return !PlayerActionFunc::isUpperVelocity(mProperty) && mCollision->isOnFloor() &&
           (mInput->isSquatButtonOn() || !mCeilingCheck->hasSpaceToStandUp());
}

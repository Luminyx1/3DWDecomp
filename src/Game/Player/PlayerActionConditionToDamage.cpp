#include "Player/PlayerActionConditionToDamage.hpp"
#include "Player/IUsePlayerDamageInvalidCheck.hpp"
#include "Player/Normal/PlayerTrigger.hpp"

/**
 * Holds on the frame the player gets hurt.
 * @param pTrigger the player's triggers
 * @param pDamageInvalidCheck damage invincibility
 */
PlayerActionConditionToDamage::PlayerActionConditionToDamage(const PlayerTrigger* pTrigger,
                                                             const IUsePlayerDamageInvalidCheck* pDamageInvalidCheck)
    : mTrigger(pTrigger), mDamageInvalidCheck(pDamageInvalidCheck) {}

/**
 * @return whether an enemy or damaging ground hurt the player this frame
 */
bool PlayerActionConditionToDamage::check() {
    if (mDamageInvalidCheck->isInvalid()) {
        return false;
    }
    if (mTrigger->isOn(PlayerTrigger::cDamage)) {
        return true;
    }
    return mTrigger->isOn(PlayerTrigger::cCollisionDamage);
}

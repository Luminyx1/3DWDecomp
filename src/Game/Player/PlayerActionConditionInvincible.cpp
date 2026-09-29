#include "Player/PlayerActionConditionInvincible.hpp"
#include "Player/IUsePlayerInvincibleCheck.hpp"

/**
 * Holds while the player is invincible.
 * @param pInvincibleCheck invincibility state
 */
PlayerActionConditionInvincible::PlayerActionConditionInvincible(const IUsePlayerInvincibleCheck* pInvincibleCheck) : mInvincibleCheck(pInvincibleCheck) {}

/**
 * @return whether the player is invincible
 */
bool PlayerActionConditionInvincible::check() {
    return mInvincibleCheck->isInvincible();
}

#include "Player/PlayerActionConditionLongFall.hpp"
#include "Player/IUsePlayerLongFallCheck.hpp"

/**
 * Holds once the player has been falling for long.
 * @param pLongFallCheck long fall check
 */
PlayerActionConditionLongFall::PlayerActionConditionLongFall(const IUsePlayerLongFallCheck* pLongFallCheck) : mLongFallCheck(pLongFallCheck) {}

/**
 * @return whether the player has been falling for long
 */
bool PlayerActionConditionLongFall::check() {
    return mLongFallCheck->isLongFalling();
}

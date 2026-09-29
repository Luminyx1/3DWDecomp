#include "Player/PlayerActionConditionCeilingCheck.hpp"
#include "Player/IUsePlayerCeilingCheck.hpp"

/**
 * Holds when there is room to stand up.
 * @param pCeilingCheck ceiling check
 */
PlayerActionConditionCeilingCheck::PlayerActionConditionCeilingCheck(const IUsePlayerCeilingCheck* pCeilingCheck) : mCeilingCheck(pCeilingCheck) {}

/**
 * @return whether the player can stand up
 */
bool PlayerActionConditionCeilingCheck::check() {
    return mCeilingCheck->hasSpaceToStandUp();
}

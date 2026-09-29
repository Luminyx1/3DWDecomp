#include "Player/PlayerActionConditionDamageToFall.hpp"
#include "Player/IUsePlayerIsDamageEnd.hpp"

/**
 * Holds once the damage reaction is over.
 * @param pDamageEnd damage action to ask
 */
PlayerActionConditionDamageToFall::PlayerActionConditionDamageToFall(const IUsePlayerIsDamageEnd* pDamageEnd) : mDamageEnd(pDamageEnd) {}

/**
 * @return whether the damage reaction is over
 */
bool PlayerActionConditionDamageToFall::check() {
    return mDamageEnd->isDamageEnd();
}

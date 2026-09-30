#include "Player/PlayerActionConditionToNormalDie.hpp"
#include "Player/IUsePlayerIsDamageEnd.hpp"
#include "Player/IUsePlayerLifeControl.hpp"

/**
 * Holds when the player is dying and any damage reaction is over.
 * @param pLifeControl the player's life
 * @param pDamageEnd damage action to wait for, or nullptr
 */
PlayerActionConditionToNormalDie::PlayerActionConditionToNormalDie(const IUsePlayerLifeControl* pLifeControl, const IUsePlayerIsDamageEnd* pDamageEnd)
    : mDamageEnd(pDamageEnd), mLifeControl(pLifeControl) {}

/**
 * @return whether to play the death
 */
bool PlayerActionConditionToNormalDie::check() {
    if (mDamageEnd != nullptr && !mDamageEnd->isDamageEnd()) {
        return false;
    }

    return mLifeControl->isDying();
}

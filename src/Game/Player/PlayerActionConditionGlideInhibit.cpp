#include "Player/PlayerActionConditionGlideInhibit.hpp"
#include "Player/IUsePlayerGlideInhibitor.hpp"

/**
 * Holds while gliding is blocked.
 * @param pGlideInhibitor glide inhibitor
 */
PlayerActionConditionGlideInhibit::PlayerActionConditionGlideInhibit(const IUsePlayerGlideInhibitor* pGlideInhibitor) : mGlideInhibitor(pGlideInhibitor) {}

/**
 * @return whether gliding is blocked
 */
bool PlayerActionConditionGlideInhibit::check() {
    return mGlideInhibitor->isInhibit();
}

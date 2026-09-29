#include "Player/PlayerActionConditionIsInhibit.hpp"
#include "Player/IUsePlayerActionInhibitor.hpp"

/**
 * Holds while an action is blocked.
 * @param pInhibitor action inhibitor
 */
PlayerActionConditionIsInhibit::PlayerActionConditionIsInhibit(const IUsePlayerActionInhibitor* pInhibitor) : mInhibitor(pInhibitor) {}

/**
 * @return whether the action is blocked
 */
bool PlayerActionConditionIsInhibit::check() {
    return mInhibitor->isInhibit();
}

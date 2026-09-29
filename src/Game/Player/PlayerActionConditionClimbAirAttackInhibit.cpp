#include "Player/PlayerActionConditionClimbAirAttackInhibit.hpp"
#include "Player/Normal/PlayerClimbAirAttackInhibitor.hpp"

/**
 * Holds while the climb air attack is blocked.
 * @param pInhibitor climb air attack inhibitor
 */
PlayerActionConditionClimbAirAttackInhibit::PlayerActionConditionClimbAirAttackInhibit(PlayerClimbAirAttackInhibitor* pInhibitor) : mInhibitor(pInhibitor) {}

/**
 * @return whether the attack is blocked
 */
bool PlayerActionConditionClimbAirAttackInhibit::check() {
    return mInhibitor->isInhibit();
}

#include "Player/PlayerActionConditionSwimSquatInhibit.hpp"
#include "Player/Normal/PlayerSwimSquatInhibitor.hpp"

/**
 * Holds while squatting in water is blocked.
 * @param pInhibitor swim squat inhibitor
 */
PlayerActionConditionSwimSquatInhibit::PlayerActionConditionSwimSquatInhibit(PlayerSwimSquatInhibitor* pInhibitor) : mInhibitor(pInhibitor) {}

/**
 * @return whether squatting is blocked
 */
bool PlayerActionConditionSwimSquatInhibit::check() {
    return mInhibitor->isInhibit();
}

#include "Player/PlayerActionConditionToTrampleJump.hpp"
#include "Player/Normal/PlayerTrigger.hpp"

/**
 * @return whether the player just trampled something
 */
bool PlayerActionConditionToTrampleJump::check() {
    return mTrigger->isOn(PlayerTrigger::cTrample);
}

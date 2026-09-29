#include "Player/PlayerActionConditionInSinkSand.hpp"
#include "Player/Normal/SinkSandControl.hpp"

/**
 * Holds while the player sinks into quicksand.
 * @param pSinkSandControl quicksand state
 */
PlayerActionConditionInSinkSand::PlayerActionConditionInSinkSand(const SinkSandControl* pSinkSandControl) : mSinkSandControl(pSinkSandControl) {}

/**
 * @return whether the player is in quicksand
 */
bool PlayerActionConditionInSinkSand::check() {
    return mSinkSandControl->isInSinkSand();
}

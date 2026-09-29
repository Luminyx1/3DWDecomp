#include "Player/PlayerActionConditionSubActionRunning.hpp"
#include "Player/IUsePlayerSubAction.hpp"

/**
 * Holds while a sub action (throw, tail attack, ...) runs.
 * @param pSubAction sub action runner
 */
PlayerActionConditionSubActionRunning::PlayerActionConditionSubActionRunning(const IUsePlayerSubAction* pSubAction) : mSubAction(pSubAction) {}

/**
 * @return whether a sub action is running
 */
bool PlayerActionConditionSubActionRunning::check() {
    return mSubAction->isRunning();
}

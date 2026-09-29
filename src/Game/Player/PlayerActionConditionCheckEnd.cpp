#include "Player/PlayerActionConditionCheckEnd.hpp"
#include "Player/IUsePlayerActionEnd.hpp"

/**
 * Holds once the current action has finished.
 * @param pActionEnd action to ask
 */
PlayerActionConditionCheckEnd::PlayerActionConditionCheckEnd(const IUsePlayerActionEnd* pActionEnd) : mActionEnd(pActionEnd) {}

/**
 * @return whether the action has finished
 */
bool PlayerActionConditionCheckEnd::check() {
    return mActionEnd->isEnd();
}

#include "Player/PlayerActionConditionRefOrg.hpp"

/**
 * Checks a condition and keeps the result.
 * @param pCondition condition to check
 */
PlayerActionConditionRefOrg::PlayerActionConditionRefOrg(PlayerActionCondition* pCondition) : mCondition(pCondition) {}

/**
 * Checks the condition and remembers the result.
 * @return whether the condition holds
 */
bool PlayerActionConditionRefOrg::check() {
    mResult = mCondition->check();
    return mResult;
}

/**
 * Sets up the checked condition.
 */
void PlayerActionConditionRefOrg::setup() {
    mCondition->setup();
}

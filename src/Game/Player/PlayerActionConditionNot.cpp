#include "Player/PlayerActionConditionNot.hpp"

/**
 * Inverts another condition.
 * @param pCondition condition to invert
 */
PlayerActionConditionNot::PlayerActionConditionNot(PlayerActionCondition* pCondition) : mCondition(pCondition) {}

/**
 * @return whether the inverted condition does not hold
 */
bool PlayerActionConditionNot::check() {
    return !mCondition->check();
}

/**
 * Sets up the inverted condition.
 */
void PlayerActionConditionNot::setup() {
    mCondition->setup();
}

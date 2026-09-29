#include "Player/PlayerActionConditionRef.hpp"
#include "Player/PlayerActionConditionRefOrg.hpp"

/**
 * Reuses the result of another condition.
 * @param pRefOrg condition whose result to reuse
 */
PlayerActionConditionRef::PlayerActionConditionRef(const PlayerActionConditionRefOrg* pRefOrg) : mRefOrg(pRefOrg) {}

/**
 * @return the result the referenced condition got
 */
bool PlayerActionConditionRef::check() {
    return mRefOrg->getResult();
}

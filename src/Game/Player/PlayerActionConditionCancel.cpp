#include "Player/PlayerActionConditionCancel.hpp"
#include "Player/IUsePlayerActionCancelable.hpp"

/**
 * Holds while the current action can be cancelled.
 * @param pCancelable action to ask
 */
PlayerActionConditionCancel::PlayerActionConditionCancel(const IUsePlayerActionCancelable* pCancelable) : mCancelable(pCancelable) {}

/**
 * @return whether the action can be cancelled
 */
bool PlayerActionConditionCancel::check() {
    return mCancelable->isPossibleToCancel();
}

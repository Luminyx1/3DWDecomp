#include "Player/PlayerActionConditionBoolDelegate.hpp"
#include "Player/IUsePlayerBoolDelegate.hpp"

/**
 * Asks a callback.
 * @param pDelegate callback to ask
 */
PlayerActionConditionBoolDelegate::PlayerActionConditionBoolDelegate(IUsePlayerBoolDelegate* pDelegate) : mDelegate(pDelegate) {}

/**
 * @return the callback's answer
 */
bool PlayerActionConditionBoolDelegate::check() {
    return mDelegate->invoke();
}

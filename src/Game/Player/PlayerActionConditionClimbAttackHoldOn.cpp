#include "Player/PlayerActionConditionClimbAttackHoldOn.hpp"
#include "Player/IUsePlayerInput.hpp"

/**
 * Holds while the climb attack button stays held.
 * @param pInput the player's input
 */
PlayerActionConditionClimbAttackHoldOn::PlayerActionConditionClimbAttackHoldOn(const IUsePlayerInput* pInput)
    : mInput(pInput) {}

/**
 * Once the button is released this stays false until the next setup.
 * @return whether the button has been held all along
 */
bool PlayerActionConditionClimbAttackHoldOn::check() {
    mIsHolding = mIsHolding && mInput->isClimbAttackButtonOn();
    return mIsHolding;
}

/**
 * Starts watching the button again.
 */
void PlayerActionConditionClimbAttackHoldOn::setup() {
    mIsHolding = true;
}

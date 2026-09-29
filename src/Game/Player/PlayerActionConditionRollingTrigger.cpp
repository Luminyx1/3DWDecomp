#include "Player/PlayerActionConditionRollingTrigger.hpp"
#include "Player/IUsePlayerInput.hpp"

/**
 * Holds on the frame the roll button is pressed.
 * @param pInput the player's input
 */
PlayerActionConditionRollingTrigger::PlayerActionConditionRollingTrigger(const IUsePlayerInput* pInput) : mInput(pInput) {}

/**
 * @return whether the roll button was just pressed
 */
bool PlayerActionConditionRollingTrigger::check() {
    return mInput->isRollingTrigOn();
}

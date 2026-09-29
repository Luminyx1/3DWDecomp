#include "Player/PlayerActionConditionStoneStatueTrig.hpp"
#include "Player/IUsePlayerInput.hpp"

/**
 * Holds on the frame the statue button (tanooki) is pressed.
 * @param pInput the player's input
 */
PlayerActionConditionStoneStatueTrig::PlayerActionConditionStoneStatueTrig(const IUsePlayerInput* pInput) : mInput(pInput) {}

/**
 * @return whether the statue button was just pressed
 */
bool PlayerActionConditionStoneStatueTrig::check() {
    return mInput->isStoneStatueTrigOn();
}

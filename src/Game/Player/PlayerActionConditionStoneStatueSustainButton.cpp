#include "Player/PlayerActionConditionStoneStatueSustainButton.hpp"
#include "Player/IUsePlayerInput.hpp"

/**
 * Holds while the statue button (tanooki) is held.
 * @param pInput the player's input
 */
PlayerActionConditionStoneStatueSustainButton::PlayerActionConditionStoneStatueSustainButton(const IUsePlayerInput* pInput) : mInput(pInput) {}

/**
 * @return whether the statue button is held
 */
bool PlayerActionConditionStoneStatueSustainButton::check() {
    return mInput->isStoneStatueSustainButtonOn();
}

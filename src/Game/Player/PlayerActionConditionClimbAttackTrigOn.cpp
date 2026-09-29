#include "Player/PlayerActionConditionClimbAttackTrigOn.hpp"
#include "Player/IUsePlayerInput.hpp"

/**
 * Holds on the frame the climb attack button is pressed.
 * @param pInput the player's input
 */
PlayerActionConditionClimbAttackTrigOn::PlayerActionConditionClimbAttackTrigOn(const IUsePlayerInput* pInput) : mInput(pInput) {}

/**
 * @return whether the climb attack button was just pressed
 */
bool PlayerActionConditionClimbAttackTrigOn::check() {
    return mInput->isClimbAttackTrigOn();
}

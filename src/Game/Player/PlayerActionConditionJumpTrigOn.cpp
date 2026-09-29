#include "Player/PlayerActionConditionJumpTrigOn.hpp"
#include "Player/IUsePlayerInput.hpp"

/**
 * Holds on the frame a jump is input.
 * @param pInput the player's input
 */
PlayerActionConditionJumpTrigOn::PlayerActionConditionJumpTrigOn(const IUsePlayerInput* pInput) : mInput(pInput) {}

/**
 * @return whether a jump was just input
 */
bool PlayerActionConditionJumpTrigOn::check() {
    return mInput->isJumpTrigOn();
}

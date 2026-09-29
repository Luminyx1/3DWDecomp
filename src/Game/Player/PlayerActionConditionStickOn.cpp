#include "Player/PlayerActionConditionStickOn.hpp"
#include "Player/IUsePlayerInput.hpp"

/**
 * Holds while the stick is tilted.
 * @param pInput the player's input
 */
PlayerActionConditionStickOn::PlayerActionConditionStickOn(const IUsePlayerInput* pInput) : mInput(pInput) {}

/**
 * @return whether the stick is tilted
 */
bool PlayerActionConditionStickOn::check() {
    return mInput->isStickOn();
}

#include "Player/PlayerActionConditionGroundMoveToSquat.hpp"
#include "Player/IUsePlayerInput.hpp"

/**
 * Holds while the squat button is held.
 * @param pInput the player's input
 */
PlayerActionConditionGroundMoveToSquat::PlayerActionConditionGroundMoveToSquat(const IUsePlayerInput* pInput) : mInput(pInput) {}

/**
 * @return whether the squat button is held
 */
bool PlayerActionConditionGroundMoveToSquat::check() {
    return mInput->isSquatButtonOn();
}

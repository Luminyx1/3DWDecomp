#include "Player/PlayerActionConditionSpin.hpp"
#include "Player/Normal/PlayerSpinJumpChecker.hpp"

/**
 * Holds when a spin is input.
 * @param pSpinJumpChecker spin input detector
 */
PlayerActionConditionSpin::PlayerActionConditionSpin(const PlayerSpinJumpChecker* pSpinJumpChecker) : mSpinJumpChecker(pSpinJumpChecker) {}

/**
 * @return whether a spin was input
 */
bool PlayerActionConditionSpin::check() {
    return mSpinJumpChecker->isSpin();
}

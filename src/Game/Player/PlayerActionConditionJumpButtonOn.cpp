#include "Player/PlayerActionConditionJumpButtonOn.hpp"
#include "Player/IUsePlayerInput.hpp"

/**
 * @return whether the jump button is held
 */
bool PlayerActionConditionJumpButtonOn::check() {
    return mInput->isJumpButtonOn();
}

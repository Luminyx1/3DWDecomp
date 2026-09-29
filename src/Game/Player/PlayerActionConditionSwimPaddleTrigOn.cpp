#include "Player/PlayerActionConditionSwimPaddleTrigOn.hpp"
#include "Player/IUsePlayerInput.hpp"

/**
 * @return whether a swim stroke was just input
 */
bool PlayerActionConditionSwimPaddleTrigOn::check() {
    return mInput->isSwimPaddleTrigOn();
}

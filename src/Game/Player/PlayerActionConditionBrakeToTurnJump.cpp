#include "Player/PlayerActionConditionBrakeToTurnJump.hpp"
#include "Player/IUsePlayerInput.hpp"
#include "Player/PlayerActionFunc.hpp"
#include "Player/Normal/PlayerProperty.hpp"

/**
 * Holds when a jump is input while the stick points backwards.
 * @param pInput the player's input
 * @param pProperty the player's physical state
 */
PlayerActionConditionBrakeToTurnJump::PlayerActionConditionBrakeToTurnJump(const IUsePlayerInput* pInput, const PlayerProperty* pProperty)
    : mInput(pInput), mProperty(pProperty) {}

/**
 * @return whether to do a side somersault
 */
bool PlayerActionConditionBrakeToTurnJump::check() {
    return PlayerActionFunc::isOppositeInput(mInput, mProperty, mProperty->getFront()) && mInput->isJumpTrigOn();
}

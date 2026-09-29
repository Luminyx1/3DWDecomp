#include "Player/PlayerActionConditionBrakeToTurn.hpp"
#include "Player/IUsePlayerActionEnd.hpp"
#include "Player/IUsePlayerInput.hpp"
#include "Player/PlayerActionFunc.hpp"
#include "Player/Normal/PlayerProperty.hpp"

/**
 * Holds when the brake is over and the stick points backwards.
 * @param pInput the player's input
 * @param pProperty the player's physical state
 * @param pActionEnd action to ask
 */
PlayerActionConditionBrakeToTurn::PlayerActionConditionBrakeToTurn(const IUsePlayerInput* pInput, const PlayerProperty* pProperty, const IUsePlayerActionEnd* pActionEnd)
    : mInput(pInput), mProperty(pProperty), mActionEnd(pActionEnd) {}

/**
 * @return whether to turn around
 */
bool PlayerActionConditionBrakeToTurn::check() {
    return mActionEnd->isEnd() && PlayerActionFunc::isOppositeInput(mInput, mProperty, mProperty->mFront);
}

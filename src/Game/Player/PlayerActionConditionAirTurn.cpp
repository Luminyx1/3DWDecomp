#include "Player/PlayerActionConditionAirTurn.hpp"
#include "Player/IUsePlayerAirTurnCheck.hpp"

/**
 * Holds while the player turns around in the air.
 * @param pAirTurnCheck air turn state
 */
PlayerActionConditionAirTurn::PlayerActionConditionAirTurn(const IUsePlayerAirTurnCheck* pAirTurnCheck) : mAirTurnCheck(pAirTurnCheck) {}

/**
 * @return whether the player is turning in the air
 */
bool PlayerActionConditionAirTurn::check() {
    return mAirTurnCheck->isAirTurning();
}

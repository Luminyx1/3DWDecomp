#include "Player/PlayerActionConditionBindSquat.hpp"
#include "Player/IUsePlayerBindEndParamGetter.hpp"

/**
 * Holds when the last bind ended with the player squatting.
 * @param pBindEndParamGetter how the last bind ended
 */
PlayerActionConditionBindSquat::PlayerActionConditionBindSquat(const IUsePlayerBindEndParamGetter* pBindEndParamGetter) : mBindEndParamGetter(pBindEndParamGetter) {}

/**
 * @return whether the bind ended in a squat
 */
bool PlayerActionConditionBindSquat::check() {
    return mBindEndParamGetter->isBindEndSquat();
}

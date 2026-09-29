#include "Player/PlayerActionConditionBindEndOnGround.hpp"
#include "Player/IUsePlayerBindEndParamGetter.hpp"

/**
 * Holds when the last bind ended with the player on the ground.
 * @param pBindEndParamGetter how the last bind ended
 */
PlayerActionConditionBindEndOnGround::PlayerActionConditionBindEndOnGround(const IUsePlayerBindEndParamGetter* pBindEndParamGetter) : mBindEndParamGetter(pBindEndParamGetter) {}

/**
 * @return whether the bind ended on the ground
 */
bool PlayerActionConditionBindEndOnGround::check() {
    return mBindEndParamGetter->isBindEndOnGround();
}

#include "Player/PlayerActionConditionBindJump.hpp"
#include "Player/IUsePlayerBindEndParamGetter.hpp"

/**
 * Holds when the last bind ended with a launch (the binder left an end parameter).
 * @param pBindEndParamGetter how the last bind ended
 */
PlayerActionConditionBindJump::PlayerActionConditionBindJump(const IUsePlayerBindEndParamGetter* pBindEndParamGetter) : mBindEndParamGetter(pBindEndParamGetter) {}

/**
 * @return whether there is a bind end parameter
 */
bool PlayerActionConditionBindJump::check() {
    return mBindEndParamGetter->getBindEndParam() != nullptr;
}

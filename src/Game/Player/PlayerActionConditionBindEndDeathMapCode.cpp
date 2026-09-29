#include "Player/PlayerActionConditionBindEndDeathMapCode.hpp"
#include "Player/IUsePlayerBindEndParamGetter.hpp"

/**
 * Holds when the last bind ended on ground that kills the player.
 * @param pBindEndParamGetter how the last bind ended
 */
PlayerActionConditionBindEndDeathMapCode::PlayerActionConditionBindEndDeathMapCode(const IUsePlayerBindEndParamGetter* pBindEndParamGetter) : mBindEndParamGetter(pBindEndParamGetter) {}

/**
 * @return whether the bind ended on deadly ground
 */
bool PlayerActionConditionBindEndDeathMapCode::check() {
    return mBindEndParamGetter->isBindEndDeathMapCode();
}

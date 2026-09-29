#include "Player/PlayerActionConditionWallInhibitAfterBind.hpp"
#include "Player/IUsePlayerBindEndParamGetter.hpp"
#include "Player/PlayerBindEndParam.hpp"

/**
 * Holds when the object that bound the player forbids wall actions afterwards.
 * @param pBindEndParamGetter how the last bind ended
 */
PlayerActionConditionWallInhibitAfterBind::PlayerActionConditionWallInhibitAfterBind(const IUsePlayerBindEndParamGetter* pBindEndParamGetter) : mBindEndParamGetter(pBindEndParamGetter) {}

/**
 * @return whether wall actions are inhibited after the bind
 */
bool PlayerActionConditionWallInhibitAfterBind::check() {
    return mBindEndParamGetter->getBindEndParam()->mIsInhibitWall;
}

#include "Player/PlayerActionConditionAirMoveToHipDrop.hpp"
#include "Player/IUsePlayerCollisionCheckArrow.hpp"
#include "Player/IUsePlayerInput.hpp"
#include "Player/Normal/PlayerConstParam.hpp"
#include "Player/Normal/PlayerProperty.hpp"

/**
 * Holds when a ground pound is input high enough above the ground.
 * @param pInput the player's input
 * @param pProperty the player's physical state
 * @param pCheckArrow line check against the map
 * @param pConstParam the player's tuning values
 */
PlayerActionConditionAirMoveToHipDrop::PlayerActionConditionAirMoveToHipDrop(
    const IUsePlayerInput* pInput, const PlayerProperty* pProperty, IUsePlayerCollisionCheckArrow* pCheckArrow,
    const PlayerConstParam* pConstParam)
    : mInput(pInput), mProperty(pProperty), mCheckArrow(pCheckArrow), mConstParam(pConstParam) {}

/**
 * Casts a line from 75 units above the player down along gravity for the ground pound height.
 * @return whether the ground pound button was pressed and no ground is that close below
 */
bool PlayerActionConditionAirMoveToHipDrop::check() {
    if (!mInput->isHipDropTrigOn()) {
        return false;
    }
    const PlayerProperty* pProperty = mProperty;
    IUsePlayerCollisionCheckArrow* pCheckArrow = mCheckArrow;
    sead::Vector3f start = pProperty->mTrans + pProperty->mUpDir * 75.0f;
    sead::Vector3f arrow = pProperty->mGravity * (mConstParam->getHipDropHeight() + 75.0f);
    return !pCheckArrow->checkArrow(start, arrow);
}

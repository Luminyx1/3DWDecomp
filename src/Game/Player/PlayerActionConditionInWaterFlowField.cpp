#include "Player/PlayerActionConditionInWaterFlowField.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Player/IUsePlayerWaterFlowField.hpp"

/**
 * Holds while a water current pushes the player.
 * @param pWaterFlowField water current
 */
PlayerActionConditionInWaterFlowField::PlayerActionConditionInWaterFlowField(const IUsePlayerWaterFlowField* pWaterFlowField)
    : mWaterFlowField(pWaterFlowField) {}

/**
 * @return whether there is a current
 */
bool PlayerActionConditionInWaterFlowField::check() {
    return !al::isNearZero(mWaterFlowField->getFlowField(), 0.001f);
}

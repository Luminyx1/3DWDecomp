#include "Player/PlayerActionConditionWaitToPivot.hpp"
#include <cmath>
#include "Library/Math/MathUtil.hpp"
#include "Player/IUsePlayerInput.hpp"
#include "Player/Normal/PlayerConstParam.hpp"
#include "Player/Normal/PlayerProperty.hpp"

/**
 * Holds when the stick points far enough away from the player's front.
 * @param pInput the player's input
 * @param pProperty the player's physical state
 * @param pConstParam the player's tuning values
 */
PlayerActionConditionWaitToPivot::PlayerActionConditionWaitToPivot(const IUsePlayerInput* pInput,
                                                                   const PlayerProperty* pProperty,
                                                                   const PlayerConstParam* pConstParam)
    : mInput(pInput), mProperty(pProperty), mConstParam(pConstParam) {}

/**
 * @return whether the stick is at least PivotDegree away from the front, along the ground
 */
bool PlayerActionConditionWaitToPivot::check() {
    sead::Vector3f moveDir = mInput->getMoveVec();
    al::verticalizeVec(&moveDir, mProperty->mGroundUp, moveDir);
    if (al::normalizeOrZero(&moveDir)) {
        return false;
    }
    f32 dot = moveDir.dot(mProperty->mFront);
    return dot <= std::cos(mConstParam->getPivotDegree() * 0.017453292f);
}

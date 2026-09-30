#include "Player/PlayerActionConditionFloorAngle.hpp"
#include <math/seadMathCalcCommon.h>
#include "Player/IUsePlayerCollision.hpp"
#include "Player/Normal/PlayerProperty.hpp"

/**
 * Holds while the slope of the floor compares to an angle.
 * @param pProperty the player's physical state
 * @param pCollision the player's collision
 * @param operationType how the slope has to compare to the angle
 * @param angle threshold in degrees
 */
PlayerActionConditionFloorAngle::PlayerActionConditionFloorAngle(const PlayerProperty* pProperty,
                                                                 const IUsePlayerCollision* pCollision,
                                                                 EOperationType operationType, f32 angle)
    : mProperty(pProperty), mCollision(pCollision), mOperationType(operationType), mAngle(angle) {}

/**
 * @return whether the player is on the floor and its angle to the up direction compares as asked
 */
bool PlayerActionConditionFloorAngle::check() {
    if (!mCollision->isOnFloor()) {
        return false;
    }

    IUsePlayerCollision::Info info = {};
    mCollision->getFloorInfo(&info);
    f32 cos = sead::Mathf::clamp(mProperty->mUpDir.dot(info.mNormal), -1.0f, 1.0f);
    f32 angle = sead::Mathf::rad2deg(sead::Mathf::acos(cos));
    switch (mOperationType) {
    case cGreater:
        return mAngle < angle;
    case cGreaterEqual:
        return mAngle <= angle;
    case cLessEqual:
        return mAngle >= angle;
    case cLess:
        return mAngle > angle;
    default:
        return false;
    }
}

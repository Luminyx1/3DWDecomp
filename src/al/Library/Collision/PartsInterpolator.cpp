#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Collision/CollisionParts.hpp"
#include "Project/Collision/CollisionPartsFilterBase.hpp"

namespace al {
/**
 * Creates a filter that uses a function to invalidate collision parts.
 * @param pFunc filter function
 * @return the filter
 */
CollisionPartsFilterBase* createCollisionPartsFilterFunc(CollisionPartsFilterFuncPtr pFunc) {
    return new CollisionPartsFilterFunc(pFunc);
}

/**
 * Creates a filter that uses a function with a sensor to invalidate collision parts.
 * @param pFunc filter function
 * @param pSensor sensor passed to the function
 * @return the filter
 */
CollisionPartsFilterBase*
createCollisionPartsFilterFuncSendMsg(CollisionPartsFilterSendMsgFuncPtr pFunc, HitSensor* pSensor) {
    return new CollisionPartsFilterFuncSendMsg(pFunc, pSensor);
}

/**
 * Creates a filter that invalidates the collision parts connected to a sensor.
 * @param pSensor sensor
 * @return the filter
 */
CollisionPartsFilterBase* createCollisionPartsFilterConnectedSensor(const HitSensor* pSensor) {
    return new CollisionPartsFilterConnectedSensor(pSensor);
}

/**
 * Checks whether the collision parts belong to the actor, or to any other actor.
 * @param rParts collision parts
 * @return true if the parts are filtered out
 */
bool CollisionPartsFilterActor::isInvalidParts(const CollisionParts& rParts) const {
    if (!rParts.mSensor) {
        return false;
    }

    if (mIsInvalidActorParts) {
        return getSensorHost(rParts.mSensor) == mActor;
    }

    return getSensorHost(rParts.mSensor) != mActor;
}

/**
 * Checks whether the collision parts are connected to the sensor.
 * @param rParts collision parts
 * @return true if the parts are filtered out
 */
bool CollisionPartsFilterConnectedSensor::isInvalidParts(const CollisionParts& rParts) const {
    if (!rParts.mSensor) {
        return false;
    }

    return rParts.mSensor == mSensor;
}

/**
 * Checks whether the collision parts are connected to a sensor of the type.
 * @param rParts collision parts
 * @return true if the parts are filtered out
 */
bool CollisionPartsFilterConnectedSensorType::isInvalidParts(const CollisionParts& rParts) const {
    if (!rParts.mSensor) {
        return false;
    }

    return isSensorType(rParts.mSensor, mSensorType);
}

/**
 * Checks whether the collision parts have another special purpose.
 * @param rParts collision parts
 * @return true if the parts are filtered out
 */
bool CollisionPartsFilterSpecialPurpose::isInvalidParts(const CollisionParts& rParts) const {
    if (!rParts.mSpecialPurpose) {
        return false;
    }

    return !isEqualString(mSpecialPurpose, rParts.mSpecialPurpose);
}

/**
 * Checks whether the collision parts have a special purpose.
 * @param rParts collision parts
 * @return true if the parts are filtered out
 */
bool CollisionPartsFilterNoSpecialPurpose::isInvalidParts(const CollisionParts& rParts) const {
    return rParts.mSpecialPurpose != nullptr;
}

/**
 * Checks whether the collision parts don't have the special purpose.
 * @param rParts collision parts
 * @return true if the parts are filtered out
 */
bool CollisionPartsFilterOnlySpecialPurpose::isInvalidParts(const CollisionParts& rParts) const {
    if (!rParts.mSpecialPurpose) {
        return true;
    }

    return !isEqualString(mSpecialPurpose, rParts.mSpecialPurpose);
}

/**
 * Checks whether both filters filter out the collision parts.
 * @param rParts collision parts
 * @return true if the parts are filtered out
 */
bool CollisionPartsFilterMergePair::isInvalidParts(const CollisionParts& rParts) const {
    return mFilterA->isInvalidParts(rParts) && mFilterB->isInvalidParts(rParts);
}

/**
 * Checks whether any filter filters out the collision parts.
 * @param rParts collision parts
 * @return true if the parts are filtered out
 */
bool CollisionPartsFilterOrPair::isInvalidParts(const CollisionParts& rParts) const {
    return mFilterA->isInvalidParts(rParts) || mFilterB->isInvalidParts(rParts);
}

/**
 * Checks the collision parts with the filter function.
 * @param rParts collision parts
 * @return true if the parts are filtered out
 */
bool CollisionPartsFilterFunc::isInvalidParts(const CollisionParts& rParts) const {
    return mFunc(rParts);
}

/**
 * Checks the collision parts with the filter function.
 * @param rParts collision parts
 * @return true if the parts are filtered out
 */
bool CollisionPartsFilterFuncSendMsg::isInvalidParts(const CollisionParts& rParts) const {
    return mFunc(rParts, mSensor);
}
}  // namespace al

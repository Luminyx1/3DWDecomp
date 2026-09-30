#include "Library/HitSensor/SensorHitGroup.hpp"

#include "Library/HitSensor/HitSensor.hpp"

namespace al {
namespace {
inline void checkHit(HitSensor* pA, HitSensor* pB) {
    if (pA->mHostActor == pB->mHostActor) {
        return;
    }
    sead::Vector3f diff = pA->mPos - pB->mPos;
    f32 radius = pA->mRadius + pB->mRadius;
    if (diff.squaredLength() >= radius * radius) {
        return;
    }
    switch (pB->mSensorType) {
    case HitSensorType::Eye:
    case HitSensorType::PlayerEye:
        break;
    default:
        pA->addHitSensor(pB);
        break;
    }
    switch (pA->mSensorType) {
    case HitSensorType::Eye:
    case HitSensorType::PlayerEye:
        break;
    default:
        pB->addHitSensor(pA);
        break;
    }
}
}  // namespace

/**
 * Constructs a sensor hit group.
 * @param maxSensors The maximum number of sensors in the group.
 * @param pName The group name.
 */
SensorHitGroup::SensorHitGroup(s32 maxSensors, const char* pName) : mMaxSensors(maxSensors) {
    mSensors = new HitSensor*[maxSensors];
    for (s32 i = 0; i < mMaxSensors; i++) {
        mSensors[i] = nullptr;
    }
}

/**
 * Adds a sensor to the group.
 * @param pSensor The sensor to add.
 */
void SensorHitGroup::add(HitSensor* pSensor) {
    mSensors[mSensorCount] = pSensor;
    mSensorCount++;
}

/**
 * Removes a sensor from the group by swapping in the last one.
 * @param pSensor The sensor to remove.
 */
void SensorHitGroup::remove(HitSensor* pSensor) {
    for (s32 i = 0; i < mSensorCount; i++) {
        if (mSensors[i] == pSensor) {
            mSensors[i] = mSensors[mSensorCount - 1];
            mSensorCount--;
            return;
        }
    }
}

/**
 * Gets a sensor by index.
 * @param idx The sensor index.
 * @return The sensor.
 */
HitSensor* SensorHitGroup::getSensor(s32 idx) const {
    return mSensors[idx];
}

/**
 * Clears the hit sensors of every sensor in the group.
 */
void SensorHitGroup::clear() const {
    for (s32 i = 0; i < mSensorCount; i++) {
        mSensors[i]->mNumSensors = 0;
    }
}

/**
 * Checks every sensor of this group against every sensor of another group.
 * @param pOther The other group.
 */
void SensorHitGroup::executeHitCheckGroup(SensorHitGroup* pOther) {
    s32 count = mSensorCount;
    for (s32 i = 0; i < count; i++) {
        HitSensor* sensor = mSensors[i];
        s32 otherCount = pOther->mSensorCount;
        for (s32 j = 0; j < otherCount; j++) {
            checkHit(sensor, pOther->mSensors[j]);
        }
    }
}

/**
 * Checks two sensors against each other and registers the hits.
 * @param pA The first sensor.
 * @param pB The second sensor.
 */
void SensorHitGroup::executeHitCheck(HitSensor* pA, HitSensor* pB) {
    checkHit(pA, pB);
}

/**
 * Checks every pair of sensors in this group.
 */
void SensorHitGroup::executeHitCheckInSameGroup() {
    s32 count = mSensorCount;
    for (s32 i = 0; i < count; i++) {
        HitSensor* sensor = mSensors[i];
        for (s32 j = i; j != count; j++) {
            checkHit(sensor, mSensors[j]);
        }
    }
}
}  // namespace al

#include "Library/HitSensor/HitSensorKeeper.hpp"

#include "Library/HitSensor/HitSensor.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Constructs a hit sensor keeper.
 * @param maxSensors The maximum number of sensors.
 */
HitSensorKeeper::HitSensorKeeper(s32 maxSensors) : mMaxSensorCount(maxSensors), mSensorCount(0) {
    mSensors = new HitSensor*[maxSensors];

    for (s32 i = 0; i < mMaxSensorCount; i++) {
        mSensors[i] = nullptr;
    }
}

/**
 * Creates a new sensor and adds it to the keeper.
 * @param pHost The actor owning the sensor.
 * @param pName The sensor name.
 * @param type The sensor type.
 * @param radius The sensor radius.
 * @param maxSensors The maximum number of sensors it can hit.
 * @param pFollowPos The position to follow, or nullptr.
 * @param pFollowMtx The matrix to follow, or nullptr.
 * @param rOffset The offset from the followed position.
 * @return The new sensor.
 */
HitSensor* HitSensorKeeper::addSensor(LiveActor* pHost, const char* pName, u32 type, f32 radius,
                                      u16 maxSensors, const sead::Vector3f* pFollowPos,
                                      const sead::Matrix34f* pFollowMtx,
                                      const sead::Vector3f& rOffset) {
    auto* sensor =
        new HitSensor(pHost, pName, type, radius, maxSensors, pFollowPos, pFollowMtx, rOffset);
    mSensors[mSensorCount] = sensor;
    mSensorCount++;
    sensor->update();
    return sensor;
}

/**
 * Updates the positions of all sensors.
 */
void HitSensorKeeper::update() {
    for (s32 i = 0; i < mSensorCount; i++) {
        mSensors[i]->update();
    }
}

/**
 * Makes every sensor's host attack the sensors it hit.
 */
void HitSensorKeeper::attackSensor() {
    for (s32 i = 0; i < mSensorCount; i++) {
        HitSensor* sensor = mSensors[i];
        sensor->trySensorSort();

        for (u32 j = 0; j < sensor->mNumSensors; j++) {
            HitSensor* other = sensor->mSensors[j];

            if (!isDead(other->mHostActor)) {
                sensor->mHostActor->attackSensor(sensor, other);
            }
        }
    }
}

/**
 * Gets a sensor by index.
 * @param idx The sensor index.
 * @return The sensor.
 */
HitSensor* HitSensorKeeper::getSensor(s32 idx) const {
    return mSensors[idx];
}

/**
 * Clears the hit sensors of every sensor.
 */
void HitSensorKeeper::clear() {
    for (s32 i = 0; i < mSensorCount; i++) {
        mSensors[i]->mNumSensors = 0;
    }
}

/**
 * Validates every sensor.
 */
void HitSensorKeeper::validate() {
    for (s32 i = 0; i < mSensorCount; i++) {
        mSensors[i]->validate();
    }
}

/**
 * Invalidates every sensor.
 */
void HitSensorKeeper::invalidate() {
    for (s32 i = 0; i < mSensorCount; i++) {
        mSensors[i]->invalidate();
    }
}

/**
 * Validates every sensor on behalf of the system.
 */
void HitSensorKeeper::validateBySystem() {
    for (s32 i = 0; i < mSensorCount; i++) {
        mSensors[i]->validateBySystem();
    }
}

/**
 * Invalidates every sensor on behalf of the system.
 */
void HitSensorKeeper::invalidateBySystem() {
    for (s32 i = 0; i < mSensorCount; i++) {
        mSensors[i]->invalidateBySystem();
    }
}

/**
 * Finds a sensor by name, or returns the only sensor if there is just one.
 * @param pName The sensor name.
 * @return The sensor, or nullptr if none has that name.
 */
HitSensor* HitSensorKeeper::getSensor(const char* pName) const {
    if (mSensorCount == 1) {
        return mSensors[0];
    }

    for (s32 i = 0; i < mSensorCount; i++) {
        if (isEqualString(mSensors[i]->mName, pName)) {
            return mSensors[i];
        }
    }

    return nullptr;
}
}  // namespace al

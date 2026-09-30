#include "Library/HitSensor/HitSensor.hpp"

#include <algorithm>
#include <nn/os.h>

#include "Library/HitSensor/SensorHitGroup.hpp"
#include "Library/LiveActor/LiveActor.hpp"

namespace al {
/**
 * Sorts the currently hit sensors with the sensor's sort function, if it has one.
 */
void HitSensor::trySensorSort() {
    if (mSortFunc != nullptr && mNumSensors >= 2) {
        std::sort(mSensors, mSensors + mNumSensors, *mSortFunc);
    }
}

/**
 * Makes the sensor follow a position.
 * @param pFollowPos The position to follow.
 */
void HitSensor::setFollowPosPtr(const sead::Vector3f* pFollowPos) {
    mFollowPos = pFollowPos;
    mFollowMtx = nullptr;
}

/**
 * Makes the sensor follow a matrix.
 * @param pFollowMtx The matrix to follow.
 */
void HitSensor::setFollowMtxPtr(const sead::Matrix34f* pFollowMtx) {
    mFollowPos = nullptr;
    mFollowMtx = pFollowMtx;
}

/**
 * Validates the sensor and clears its hit sensors.
 */
void HitSensor::validate() {
    if (!mIsValid) {
        mIsValid = true;

        if (mMaxSensors != 0 && mIsValidBySystem) {
            mHitGroup->add(this);
        }
    }

    mNumSensors = 0;
}

/**
 * Invalidates the sensor and clears its hit sensors.
 */
void HitSensor::invalidate() {
    if (mIsValid) {
        mIsValid = false;

        if (mMaxSensors != 0 && mIsValidBySystem) {
            mHitGroup->remove(this);
        }
    }

    mNumSensors = 0;
}

/**
 * Validates the sensor on behalf of the system and clears its hit sensors.
 */
void HitSensor::validateBySystem() {
    if (mIsValidBySystem) {
        return;
    }

    if (mMaxSensors != 0 && mIsValid) {
        mHitGroup->add(this);
    }

    mIsValidBySystem = true;
    mNumSensors = 0;
}

/**
 * Invalidates the sensor on behalf of the system and clears its hit sensors.
 */
void HitSensor::invalidateBySystem() {
    if (!mIsValidBySystem) {
        return;
    }

    if (mMaxSensors != 0 && mIsValid) {
        mHitGroup->remove(this);
    }

    mIsValidBySystem = false;
    mNumSensors = 0;
}

/**
 * Constructs a hit sensor.
 * @param pHost The actor owning the sensor.
 * @param pName The sensor name.
 * @param type The sensor type.
 * @param radius The sensor radius.
 * @param maxSensors The maximum number of sensors it can hit.
 * @param pFollowPos The position to follow, or nullptr.
 * @param pFollowMtx The matrix to follow, or nullptr.
 * @param rOffset The offset from the followed position.
 */
HitSensor::HitSensor(LiveActor* pHost, const char* pName, u32 type, f32 radius, u16 maxSensors,
                     const sead::Vector3f* pFollowPos, const sead::Matrix34f* pFollowMtx,
                     const sead::Vector3f& rOffset)
    : mName(pName), mSensorType(static_cast<HitSensorType>(type)), mRadius(radius), mMaxSensors(maxSensors), mHostActor(pHost),
      mFollowPos(pFollowPos), mFollowMtx(pFollowMtx), mFollowPosOffset(rOffset) {
    if (maxSensors != 0) {
        mSensors = new HitSensor*[maxSensors];

        for (s32 i = 0; i < mMaxSensors; i++) {
            mSensors[i] = nullptr;
        }
    }
}

/**
 * Updates the sensor position from the followed position or matrix.
 */
void HitSensor::update() {
    if (mFollowPos != nullptr) {
        const sead::Matrix34f* baseMtx = mHostActor->getBaseMtx();

        if (baseMtx != nullptr) {
            mPos.setRotated(*baseMtx, mFollowPosOffset);
            mPos += *mFollowPos;
        } else {
            mPos.setAdd(*mFollowPos, mFollowPosOffset);
        }
    } else if (mFollowMtx != nullptr) {
        mPos.setMul(*mFollowMtx, mFollowPosOffset);
    }
}

/**
 * Adds a sensor to the list of hit sensors if there is room.
 * @param pSensor The sensor that was hit.
 */
void HitSensor::addHitSensor(HitSensor* pSensor) {
    if (mNumSensors < mMaxSensors) {
        mSensors[mNumSensors] = pSensor;
        mNumSensors++;
    }
}

/**
 * Records the current system tick.
 */
void HitSensor::setTime() {
    mTime = nn::os::GetSystemTick().GetInt64Value();
}
}  // namespace al

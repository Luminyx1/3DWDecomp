#include "Library/Actor/ActorSensorController.hpp"

#include "Library/HitSensor/HitSensor.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"

namespace al {
/**
 * Constructs a controller that remembers the original radius and offset of a sensor.
 * @param pActor The actor owning the sensor.
 * @param pSensorName The sensor name.
 */
ActorSensorController::ActorSensorController(LiveActor* pActor, const char* pSensorName) {
    mSensor = getHitSensor(pActor, pSensorName);
    mSensorRadius = mSensor->mRadius;
    mFollowPosOffs = mSensor->mFollowPosOffset;
}

/**
 * Scales the sensor's radius and offset relative to their original values.
 * @param scale The scale.
 */
void ActorSensorController::setSensorScale(f32 scale) {
    setSensorRadius(mSensorRadius * scale);
    mSensor->mFollowPosOffset.set(mFollowPosOffs.x * scale, mFollowPosOffs.y * scale,
                                  mFollowPosOffs.z * scale);
}

/**
 * Sets the sensor's radius.
 * @param radius The new radius.
 */
void ActorSensorController::setSensorRadius(f32 radius) {
    mSensor->mRadius = radius;
}

/**
 * Sets the sensor's follow offset.
 * @param rOffset The new offset.
 */
void ActorSensorController::setSensorFollowPosOffset(const sead::Vector3f& rOffset) {
    mSensor->mFollowPosOffset.e = rOffset.e;
}

/**
 * Restores the sensor's original radius and offset.
 */
void ActorSensorController::resetActorSensorController() {
    HitSensor* sensor = mSensor;
    sensor->mRadius = mSensorRadius;
    mSensor->mFollowPosOffset.e = mFollowPosOffs.e;
}

/**
 * Constructs a sensor controller list.
 * @param maxControllers The maximum number of controllers.
 */
ActorSensorControllerList::ActorSensorControllerList(s32 maxControllers)
    : mMaxControllers(maxControllers) {
    mSensorControllers = new ActorSensorController*[maxControllers];

    for (s32 i = 0; i < mMaxControllers; i++) {
        mSensorControllers[i] = nullptr;
    }
}

/**
 * Adds a controller for a sensor of an actor.
 * @param pActor The actor.
 * @param pSensorName The sensor name.
 */
void ActorSensorControllerList::addSensor(LiveActor* pActor, const char* pSensorName) {
    auto* controller = new ActorSensorController(pActor, pSensorName);
    mSensorControllers[mNumControllers++] = controller;
}

/**
 * Scales every controlled sensor.
 * @param scale The scale.
 */
void ActorSensorControllerList::setAllSensorScale(f32 scale) {
    for (s32 i = 0; i < mNumControllers; i++) {
        mSensorControllers[i]->setSensorScale(scale);
    }
}

/**
 * Restores every controlled sensor.
 */
void ActorSensorControllerList::resetAllActorSensorController() {
    for (s32 i = 0; i < mNumControllers; i++) {
        mSensorControllers[i]->resetActorSensorController();
    }
}
}  // namespace al

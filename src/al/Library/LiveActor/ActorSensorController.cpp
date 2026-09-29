#include "Library/Actor/ActorSensorController.hpp"
#include "Library/HitSensor/HitSensor.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"

namespace al {
    /**
     * @brief Creates a controller for one of an actor's hit sensors and remembers its original size.
     * @param pActor The actor that owns the sensor.
     * @param pSensorName The name of the sensor.
     */
    ActorSensorController::ActorSensorController(LiveActor* pActor, const char* pSensorName) {
        HitSensor* pSensor = getHitSensor(pActor, pSensorName);
        mSensor = pSensor;
        mSensorRadius = pSensor->mRadius;
        mFollowPosOffs = pSensor->mFollowPosOffset;
    }

    /**
     * @brief Scales the sensor's original radius and offset.
     * @param scale The scale to apply to the original values.
     */
    void ActorSensorController::setSensorScale(f32 scale) {
        mSensor->mRadius = mSensorRadius * scale;
        mSensor->mFollowPosOffset.set(mFollowPosOffs * scale);
    }

    /**
     * @brief Sets the sensor's radius.
     * @param radius The new radius.
     */
    void ActorSensorController::setSensorRadius(f32 radius) {
        mSensor->mRadius = radius;
    }

    /**
     * @brief Sets the sensor's offset from the position it follows.
     * @param rOffset The new offset.
     */
    void ActorSensorController::setSensorFollowPosOffset(const sead::Vector3f& rOffset) {
        mSensor->mFollowPosOffset.set(rOffset);
    }

    /** @brief Restores the sensor's original radius and offset. */
    void ActorSensorController::resetActorSensorController() {
        HitSensor* pSensor = mSensor;
        pSensor->mRadius = mSensorRadius;
        mSensor->mFollowPosOffset.set(mFollowPosOffs);
    }

    /**
     * @brief Creates an empty controller list.
     * @param maxControllers The maximum number of sensors the list can control.
     */
    ActorSensorControllerList::ActorSensorControllerList(s32 maxControllers) : mMaxControllers(maxControllers) {
        mSensorControllers = new ActorSensorController*[maxControllers];

        for (s32 i = 0; i < mMaxControllers; i++) {
            mSensorControllers[i] = nullptr;
        }
    }

    /**
     * @brief Adds a controller for one of an actor's hit sensors.
     * @param pActor The actor that owns the sensor.
     * @param pSensorName The name of the sensor.
     */
    void ActorSensorControllerList::addSensor(LiveActor* pActor, const char* pSensorName) {
        mSensorControllers[mNumControllers++] = new ActorSensorController(pActor, pSensorName);
    }

    /**
     * @brief Scales the original radius and offset of every controlled sensor.
     * @param scale The scale to apply to the original values.
     */
    void ActorSensorControllerList::setAllSensorScale(f32 scale) {
        for (s32 i = 0; i < mNumControllers; i++) {
            mSensorControllers[i]->setSensorScale(scale);
        }
    }

    /** @brief Restores the original radius and offset of every controlled sensor. */
    void ActorSensorControllerList::resetAllActorSensorController() {
        for (s32 i = 0; i < mNumControllers; i++) {
            mSensorControllers[i]->resetActorSensorController();
        }
    }
};

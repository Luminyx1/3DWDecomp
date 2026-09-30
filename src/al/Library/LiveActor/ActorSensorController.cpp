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


}  // namespace al

#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class LiveActor;
class HitSensor;

class ActorSensorController {
public:
    ActorSensorController(LiveActor* pActor, const char* pSensorName);

    void setSensorScale(f32 scale);
    void setSensorRadius(f32 radius);
    void setSensorFollowPosOffset(const sead::Vector3f& rOffset);
    void resetActorSensorController();

    HitSensor* mSensor = nullptr;
    f32 mSensorRadius = 0.0f;
    sead::Vector3f mFollowPosOffs = {0.0f, 0.0f, 0.0f};
};

class ActorSensorControllerList {
public:
    ActorSensorControllerList(s32 maxControllers);

    void addSensor(LiveActor* pActor, const char* pSensorName);
    void setAllSensorScale(f32 scale);
    void resetAllActorSensorController();

    ActorSensorController** mSensorControllers;
    s32 mMaxControllers;
    s32 mNumControllers = 0;
};
}  // namespace al

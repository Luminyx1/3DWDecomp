#pragma once

#include <container/seadPtrArray.h>

namespace al {
class ActorInitInfo;
class LiveActor;
class HitSensor;
class JointSpringController;
}

namespace KoopaLastFunction {
void explosionCollision(al::LiveActor* pActor, const char* pName);
void initActorKoopaLastCommon(al::LiveActor* pActor, const al::ActorInitInfo& rInfo,
                              const char* pName,
                              sead::PtrArray<al::JointSpringController>* pControllers);
void attackSensorCommon(al::HitSensor* pSelf, al::HitSensor* pOther);
}

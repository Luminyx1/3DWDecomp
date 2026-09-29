#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class ActorInitInfo;
class LiveActor;
class ScreenPointer;
class ScreenPointTarget;
class SensorMsg;

ScreenPointTarget* addScreenPointTarget(LiveActor*, const ActorInitInfo&, const char*, f32,
                                        const char*, const sead::Vector3f&);
bool hitCheckSegmentScreenPointTarget(ScreenPointer*, const sead::Vector3f&, const sead::Vector3f&);
bool hitCheckScreenCircleScreenPointTarget(ScreenPointer*, const sead::Vector2f&, f32);
bool sendMsgScreenPointTarget(const SensorMsg&, ScreenPointer*, ScreenPointTarget*);
bool sendMsgScreenPointTargetSM(const SensorMsg&, ScreenPointer*, ScreenPointTarget*);
}  // namespace al

namespace alScreenPointFunction {
void updateScreenPointAll(al::LiveActor*);
}  // namespace alScreenPointFunction

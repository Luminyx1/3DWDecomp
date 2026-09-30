#pragma once

#include <math/seadVector.h>

namespace al {
class ActorInitInfo;
class LiveActor;
class ScreenPointer;
class ScreenPointTarget;
class SensorMsg;

ScreenPointTarget* addScreenPointTarget(LiveActor* pActor, const ActorInitInfo& rInfo,
                                        const char* pName, f32 radius, const char* pJointName,
                                        const sead::Vector3f& rOffset);
bool hitCheckSegmentScreenPointTarget(ScreenPointer* pPointer, const sead::Vector3f& rStart,
                                      const sead::Vector3f& rEnd);
bool hitCheckScreenCircleScreenPointTarget(ScreenPointer* pPointer, const sead::Vector2f& rPos,
                                           f32 radius);
bool sendMsgScreenPointTarget(const SensorMsg& rMsg, ScreenPointer* pPointer,
                              ScreenPointTarget* pTarget);
bool sendMsgScreenPointTargetSM(const SensorMsg& rMsg, ScreenPointer* pPointer,
                                ScreenPointTarget* pTarget);
}  // namespace al

namespace alScreenPointFunction {
void updateScreenPointAll(al::LiveActor* pActor);
}

#pragma once

#include "Library/LiveActor/ActorParamHolder.hpp"

namespace al {
class LiveActor;

const ActorParamF32* findActorParamF32(const LiveActor* pActor, const char* pName);
const ActorParamS32* findActorParamS32(const LiveActor* pActor, const char* pName);
const ActorParamMove* findActorParamMove(const LiveActor* pActor, const char* pName);
const ActorParamJump* findActorParamJump(const LiveActor* pActor, const char* pName);
const ActorParamSight* findActorParamSight(const LiveActor* pActor, const char* pName);
const ActorParamRebound* findActorParamRebound(const LiveActor* pActor, const char* pName);
void setActorParamMove(ActorParamMove* pParam, f32 moveAccel, f32 gravity, f32 moveFriction,
                       f32 turnSpeedDegree);
}  // namespace al

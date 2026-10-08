#pragma once

#include <math/seadVector.h>

namespace al {
class LiveActor;
struct ActorInitInfo;
}

namespace rc {
void initEchoEmitterHolder(const al::LiveActor* pActor, const al::ActorInitInfo& rInfo);
bool emitEcho(const al::LiveActor* pActor, const sead::Vector3f& rPos, f32 radius, s32 frame,
              bool isForce);
}

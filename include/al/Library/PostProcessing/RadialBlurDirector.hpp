#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class LiveActor;

void emitRadialBlur(const LiveActor* pActor, const sead::Vector3f& rPos, f32 radiusBegin,
                    f32 radiusEnd, s32 frame, s32 viewIndex);
}  // namespace al

#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace al {
class IUseCamera;
class SceneCameraInfo;

void calcScreenPosFromWorldPos(sead::Vector2f*, const IUseCamera*, const sead::Vector3f&, s32);
f32 calcScreenRadiusFromWorldRadius(const sead::Vector3f&, const IUseCamera*, f32);
}  // namespace al

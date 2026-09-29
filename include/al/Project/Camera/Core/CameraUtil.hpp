#pragma once

#include <math/seadVector.h>

namespace al {
class IUseCamera;
class IUseCamera_RS;
class SceneCameraInfo;

const sead::Vector3f& getCameraPos(const IUseCamera*);

s32 getViewNumMax(const IUseCamera_RS*);
s32 getViewNumMax(const SceneCameraInfo*);
bool isValidView(const IUseCamera_RS*, s32);
bool isValidView(const SceneCameraInfo*, s32);
const sead::Vector3f& getCameraAt_RS(const IUseCamera_RS*, s32);
const sead::Vector3f& getCameraAt_RS(const SceneCameraInfo*, s32);

}  // namespace al

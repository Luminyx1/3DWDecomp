#pragma once

#include <gfx/seadCamera.h>
#include <math/seadVector.h>

namespace al {
class ByamlIter;

class CameraAngleSwingInfo {
public:
    CameraAngleSwingInfo();

    void load(const ByamlIter& rIter);
    void update(const sead::Vector2f& rStick, f32 sensitivityScale);
    void makeLookAtCamera(sead::LookAtCamera* pCamera) const;

    bool isInvalidSwing = false;
    sead::Vector2f currentAngle = {0.0f, 0.0f};
    f32 maxSwingDegreeH = 15.0f;
    f32 maxSwingDegreeV = 15.0f;
    f32 targetLerpRate = 0.3f;
    f32 angleLerpRate = 0.1f;
};

}  // namespace al

#pragma once

#include <basis/seadTypes.h>

namespace al {

struct CameraObjectRequestInfo {
    bool isStopVerticalAbsorb = false;
    bool isResetPosition = false;
    bool isResetAngleV = false;
    bool isDownToDefaultAngleBySpeed = false;
    bool isUpToTargetAngleBySpeed = false;
    f32 targetAngleV = 0.0f;
    f32 angleSpeed = 0.0f;
    bool isMoveDownAngle = false;
    bool isSetAngleV = false;
    f32 angleV = 23.0f;
};

}  // namespace al

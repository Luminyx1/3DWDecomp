#pragma once

#include <basis/seadTypes.h>

#include "Library/Camera/CameraTicket.hpp"

namespace al {

struct CameraStartInfo {
    void setAreaAngleH(f32 angle) {
        isExistAreaAngleH = true;
        areaAngleH = angle;
    }

    void setAreaAngleV(f32 angle) {
        isExistAreaAngleV = true;
        areaAngleV = angle;
    }

    CameraTicket::Priority prePriorityType = static_cast<CameraTicket::Priority>(-1);
    const char* preCameraName = nullptr;
    f32 preCameraSwingAngleH = 0.0f;
    f32 preCameraSwingAngleV = 0.0f;
    f32 preCameraMaxSwingAngleH = 0.0f;
    f32 preCameraMaxSwingAngleV = 0.0f;
    bool isInvalidCollidePreCamera = false;
    bool isInvalidKeepPreCameraDistance = false;
    bool isInvalidKeepPreCameraDistanceIfNoCollide = false;
    bool isValidResetPreCameraPose = false;
    bool isValidKeepPreSelfCameraPose = false;
    bool _25 = false;
    bool isExistAreaAngleH = false;
    f32 areaAngleH = 0.0f;
    bool isExistAreaAngleV = false;
    f32 areaAngleV = 0.0f;
    bool isExistNextPoseByPreCamera = false;
    f32 nextAngleHByPreCamera = 0.0f;
    f32 nextAngleVByPreCamera = 0.0f;
};

static_assert(sizeof(CameraStartInfo) == 0x40);

}  // namespace al

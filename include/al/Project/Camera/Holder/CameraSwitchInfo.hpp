#pragma once

#include "Library/Camera/CameraPoseInfo.hpp"

namespace al {
class CameraTicket;

struct CameraSwitchInfo {
    CameraSwitchInfo();

    void reset();

    CameraTicket* ticket = nullptr;
    s32 interpoleStep = -1;
    bool isKeepPose = false;
    bool isSetNextPoseInfo = false;
    CameraPoseInfo nextPoseInfo = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
};

}  // namespace al

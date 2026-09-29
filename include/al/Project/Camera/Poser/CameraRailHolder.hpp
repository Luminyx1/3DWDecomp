#pragma once

#include <basis/seadTypes.h>

namespace al {
class CameraRail;
class CameraRailInfo;

/// Holds the camera rails of a scene.
class CameraRailHolder {
public:
    CameraRailHolder();

    void setCameraRail(CameraRail* pRail, bool isValid);
    CameraRail* getCameraRail(s32 index) const;
    void setCameraRailValidFlag(s32 index, bool isValid);
    bool isCameraRailValid(s32 index) const;

    s32 mNumRails = 0;                  // _0
    CameraRailInfo* mRailInfos = nullptr;  // _8
};
}  // namespace al

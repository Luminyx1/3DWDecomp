#pragma once

#include <basis/seadTypes.h>

namespace al {
class CameraRail;

class CameraRailInfo {
public:
    CameraRailInfo();

    CameraRail* mRail = nullptr;
    bool mIsValid = true;
};

static_assert(sizeof(CameraRailInfo) == 0x10);

class CameraRailHolder {
public:
    CameraRailHolder();

    void setCameraRail(CameraRail* pRail, bool isValid);
    CameraRail* getCameraRail(s32 index) const;
    void setCameraRailValidFlag(s32 index, bool isValid);
    bool isCameraRailValid(s32 index) const;

    s32 mCameraRailNum = 0;
    CameraRailInfo* mCameraRailInfos = nullptr;
};

static_assert(sizeof(CameraRailHolder) == 0x10);
}  // namespace al

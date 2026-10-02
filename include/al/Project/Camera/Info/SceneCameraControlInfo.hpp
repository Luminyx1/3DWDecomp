#pragma once

#include <math/seadVector.h>

namespace al {
class CameraCreator;
class CameraHolder;
class CameraRailHolder;
class CameraSwitcher;
class LiveActor;

struct SceneCameraControlInfo {
    SceneCameraControlInfo();

    LiveActor* mTopPlayerActor = nullptr;
    LiveActor* mTopPlayerActorFromRail = nullptr;
    sead::Vector3f* mPlayerRailPos = nullptr;
    sead::Vector3f mPlayerRailDir = sead::Vector3f::zero;
    const sead::Vector3f** mLookAtPosPtrs = nullptr;
    bool mIsRequestCancelInterpole = true;
    bool mIsRequestResetUserControl;
    bool* mIsCalcTarget = nullptr;
    const char* mShakeName = nullptr;
    CameraHolder* mCameraHolder = nullptr;
    CameraCreator* mCameraCreator = nullptr;
    CameraSwitcher* mCameraSwitcher = nullptr;
    CameraRailHolder* mCameraRailHolder = nullptr;
};

static_assert(sizeof(SceneCameraControlInfo) == 0x68);

}  // namespace al

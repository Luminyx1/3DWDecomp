#pragma once

#include <basis/seadTypes.h>

namespace al {
class CameraRequestParamHolder;
class CameraSwitchRequester;
class SpecialCameraHolder;

class PauseCameraCtrl {
public:
    PauseCameraCtrl(f32 fovyDegree);

    bool isCameraPause() const { return mIsCameraPause; }

    f32 getFovyDegree() const { return mFovyDegree; }

private:
    bool mIsCameraPause = false;
    f32 mFovyDegree;
};

class SceneCameraViewCtrl {
public:
    SceneCameraViewCtrl();

    CameraSwitchRequester* getRequester() const { return mRequester; }

    const char* getShakeName() const { return mShakeName; }

    void setShakeName(const char* pName) { mShakeName = pName; }

private:
    CameraSwitchRequester* mRequester = nullptr;
    const char* mName = "Start";
    const char* mShakeName = nullptr;
};

class SceneCameraCtrl {
public:
    SceneCameraCtrl();

    void init(s32 viewNum);

    SceneCameraViewCtrl* getViewCtrl(s32 index) const { return &mViewCtrls[index]; }

    CameraRequestParamHolder* getRequestParamHolder() const { return mRequestParamHolder; }

private:
    s32 mViewNum = 0;
    SceneCameraViewCtrl* mViewCtrls = nullptr;
    CameraRequestParamHolder* mRequestParamHolder = nullptr;
    SpecialCameraHolder* mSpecialCameraHolder = nullptr;
};

}  // namespace al

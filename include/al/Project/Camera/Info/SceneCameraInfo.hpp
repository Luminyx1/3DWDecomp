#pragma once

#include <basis/seadTypes.h>
#include <gfx/seadCamera.h>
#include <math/seadMatrix.h>

namespace al {
class CameraViewInfo;
class Projection;
struct SceneCameraControlInfo;

class SceneCameraInfo {
public:
    SceneCameraInfo(s32 viewNum);

    void initViewInfo(CameraViewInfo* pViewInfo);
    const char* getViewName(s32 index) const;

    s32 getViewNumMax() const { return mViewNumMax; }

    CameraViewInfo* getViewAt(s32 index) const { return mViewArray[index]; }

    const sead::Matrix34f* mViewMtx = nullptr;
    void* _8 = nullptr;
    void* _10 = nullptr;
    void* _18 = nullptr;
    sead::LookAtCamera* mLookAtCamera = nullptr;
    void* _28 = nullptr;
    sead::Projection* mProjection = nullptr;
    sead::Projection* _38 = nullptr;
    SceneCameraControlInfo* mControlInfo = nullptr;
    s32 mViewNumMax;
    CameraViewInfo** mViewArray;
};

}  // namespace al

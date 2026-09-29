#pragma once

#include <basis/seadTypes.h>

namespace al {
class CameraViewInfo;

/// Holds the camera views of a scene.
class SceneCameraInfo {
public:
    SceneCameraInfo(s32 viewNumMax);

    void initViewInfo(CameraViewInfo* pViewInfo);
    const char* getViewName(s32 index) const;

    s32 getViewNumMax() const { return mViewNumMax; }
    CameraViewInfo* getViewAt(s32 index) const { return mViewArray[index]; }

    void* _0 = nullptr;                   // _0
    void* _8 = nullptr;                   // _8
    void* _10 = nullptr;                  // _10
    void* _18 = nullptr;                  // _18
    void* _20 = nullptr;                  // _20
    void* _28 = nullptr;                  // _28
    void* _30 = nullptr;                  // _30
    void* _38 = nullptr;                  // _38
    void* _40 = nullptr;                  // _40
    s32 mViewNumMax;                      // _48
    CameraViewInfo** mViewArray;          // _50
};
}  // namespace al

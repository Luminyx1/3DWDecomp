#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace al {
class SceneCameraInfo;

class EffectCameraHolder {
public:
    EffectCameraHolder();

    void setSceneCameraInfo(SceneCameraInfo* pInfo);
    const sead::Matrix34f* getViewMtxPtr() const;
    f32 getFovy() const;
    const sead::Vector3f& getCameraPos() const;
    void calcScreenPosFromWorldPos(sead::Vector2f* pScreenPos, const sead::Vector3f& rPos) const;
    f32 calcFollowScaleByFovy() const;
    f32 calcFarClipRateByFovy() const;
    bool tryMakeBillboardMtx(sead::Matrix34f* pMtx, const sead::Vector3f& rPos) const;
    bool tryMakeYBillboardMtx(sead::Matrix34f* pMtx, const sead::Vector3f& rPos) const;
    bool tryMakeCameraFrontMtx(sead::Matrix34f* pMtx) const;

    SceneCameraInfo* getSceneCameraInfo() const { return mSceneCameraInfo; }

private:
    SceneCameraInfo* mSceneCameraInfo;
};
}  // namespace al

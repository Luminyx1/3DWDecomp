#pragma once

#include <math/seadVector.h>

#include "Project/Camera/Core/IUseCameraDirector.hpp"
#include "Project/Camera/Main/IUseCameraDirector_RS.hpp"

namespace al {
class CameraDirector_RS;
class ClippingFarAreaObserver;
class FrustumRadar;
class PlayerHolder;
class SceneCameraInfo;

class ClippingJudge : public IUseCamera, public IUseCamera_RS {
public:
    ClippingJudge(const ClippingFarAreaObserver* pFarAreaObserver,
                  SceneCameraInfo* pSceneCameraInfo, CameraDirector_RS* pCameraDirector);

    void update();
    void setPlayerPos(const PlayerHolder* pPlayerHolder);
    f32 getFarClipping() const;
    f32 getNearClipping() const;
    bool isJudgedToClipFrustumUnUseFarLevel(const sead::Vector3f& rPos, f32 radius,
                                            f32 near) const;
    bool judgeInAreaCore(const sead::Vector3f& rPos, f32 radius, f32 near) const;
    bool isJudgedToClipFrustum(const sead::Vector3f& rPos, f32 radius, f32 near,
                               s32 farLevel) const;
    bool judgeInAreaCore(const sead::Vector3f& rPos, f32 radius, f32 near, f32 far) const;
    bool isJudgedToClipFrustum(const sead::Vector3f* pPoints, s32 numPoints, f32 near,
                               s32 farLevel) const;
    bool judgeInAreaCore(const sead::Vector3f* pPoints, s32 numPoints, f32 near, f32 far) const;
    bool judgeInAreaCore(const sead::Vector3f* pPoints, s32 numPoints, f32 near) const;

    void setUseClippingPosAsPlayerPos(bool isUse) { mIsUseClippingPosAsPlayerPos = isUse; }

    SceneCameraInfo* getSceneCameraInfo() const override { return mSceneCameraInfo; }

    CameraDirector_RS* getCameraDirector_RS() const override { return mCameraDirector; }

    const ClippingFarAreaObserver* mFarAreaObserver;
    FrustumRadar* mFrustumRadar = nullptr;
    SceneCameraInfo* mSceneCameraInfo;
    CameraDirector_RS* mCameraDirector;
    sead::Vector3f mPlayerPos;
    sead::Vector3f mCameraPos;
    bool mIsUseClippingPosAsPlayerPos = false;
};
}  // namespace al

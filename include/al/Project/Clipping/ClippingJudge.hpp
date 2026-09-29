#pragma once

#include <math/seadVector.h>
#include "Project/Camera/Core/IUseCameraDirector.hpp"
#include "Project/Camera/Main/IUseCameraDirector_RS.hpp"

namespace al {
    class ClippingFarAreaObserver;
    class FrustumRadar;
    class PlayerHolder;

    /// Judges whether positions are clipped by the camera's view frustum.
    class ClippingJudge : public IUseCamera, public IUseCamera_RS {
    public:
        ClippingJudge(const ClippingFarAreaObserver* pFarAreaObserver, SceneCameraInfo* pSceneCameraInfo,
                      CameraDirector_RS* pCameraDirector);

        void update();
        void setPlayerPos(const PlayerHolder* pPlayerHolder);
        f32 getFarClipping() const;
        f32 getNearClipping() const;
        bool isJudgedToClipFrustumUnUseFarLevel(const sead::Vector3f& rPos, f32 radius, f32 near) const;
        bool judgeInAreaCore(const sead::Vector3f& rPos, f32 radius, f32 near) const;
        bool isJudgedToClipFrustum(const sead::Vector3f& rPos, f32 radius, f32 near, s32 farLevel) const;
        bool judgeInAreaCore(const sead::Vector3f& rPos, f32 radius, f32 near, f32 far) const;
        bool isJudgedToClipFrustum(const sead::Vector3f* pPoints, s32 numPoints, f32 near, s32 farLevel) const;
        bool judgeInAreaCore(const sead::Vector3f* pPoints, s32 numPoints, f32 near, f32 far) const;
        bool judgeInAreaCore(const sead::Vector3f* pPoints, s32 numPoints, f32 near) const;

        void setIsUseClippingPosAsPlayerPos(bool isUse) { mIsUseClippingPosAsPlayerPos = isUse; }

        virtual SceneCameraInfo* getSceneCameraInfo() const override { return mSceneCameraInfo; }

        virtual CameraDirector_RS* getCameraDirector_RS() const override { return mCameraDirector; }

        const ClippingFarAreaObserver* mFarAreaObserver;    // _10
        FrustumRadar* mFrustumRadar;                        // _18
        SceneCameraInfo* mSceneCameraInfo;                  // _20
        CameraDirector_RS* mCameraDirector;                 // _28
        sead::Vector3f mPlayerPos;                          // _30
        sead::Vector3f mCameraPos;                          // _3C
        bool mIsUseClippingPosAsPlayerPos;                  // _48
    };
};

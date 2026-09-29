#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace al {
    /// Judges whether spheres or points are inside a view frustum.
    class FrustumRadar {
    public:
        FrustumRadar();

        void calcFrustumArea(const sead::Matrix34f& rViewMtx, f32 fovy, f32 aspect, f32 near, f32 far);
        void setLocalAxis(const sead::Matrix34f& rViewMtx);
        void setFactor(f32 fovy, f32 aspect);
        void calcFrustumArea(const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx, f32 near, f32 far);
        void setFactor(const sead::Matrix44f& rProjMtx);
        void calcFrustumAreaStereo(const sead::Matrix34f& rViewMtxLeft, const sead::Matrix34f& rViewMtxRight,
                                   const sead::Matrix44f& rProjMtx, f32 near, f32 far);
        void setLocalAxisStereo(const sead::Matrix34f& rViewMtxLeft, const sead::Matrix34f& rViewMtxRight);
        void setFactorStereo(const sead::Matrix44f& rProjMtx);
        bool judgeInLeft(const sead::Vector3f& rPos, f32 radius) const;
        bool judgeInRight(const sead::Vector3f& rPos, f32 radius) const;
        bool judgeInTop(const sead::Vector3f& rPos, f32 radius) const;
        bool judgeInBottom(const sead::Vector3f& rPos, f32 radius) const;
        bool judgeInNear(const sead::Vector3f& rPos, f32 radius, f32 near) const;
        bool judgeInFar(const sead::Vector3f& rPos, f32 radius, f32 far) const;
        bool judgeInArea(const sead::Vector3f& rPos, f32 radius, f32 near, f32 far) const;
        bool judgeInArea(const sead::Vector3f* pPoints, s32 numPoints, f32 near, f32 far) const;
        bool judgeInArea(const sead::Vector3f& rPos, f32 radius, f32 near) const;
        bool judgeInArea(const sead::Vector3f& rPos, f32 radius) const;
        bool judgeInAreaNoFar(const sead::Vector3f& rPos, f32 radius) const;

        f32 getNear() const { return mNear; }

        sead::Vector3f mOrthoSide;          // _0
        sead::Vector3f mOrthoUp;            // _C
        sead::Vector3f mOrthoFront;         // _18
        sead::Vector3f mOrthoTrans;         // _24
        f32 mHorizontalSlope;               // _30
        f32 mHorizontalNormFactor;          // _34
        f32 mVerticalSlope;                 // _38
        f32 mVerticalNormFactor;            // _3C
        f32 mStereoEyeOffset;               // _40
        f32 mStereoSlopeLeft;               // _44
        f32 mStereoNormFactorLeft;          // _48
        f32 mStereoSlopeRight;              // _4C
        f32 mStereoNormFactorRight;         // _50
        f32 mNear;                          // _54
        f32 mFar;                           // _58
    };
};

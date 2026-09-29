#include "Project/Clipping/FrustumRadar.hpp"
#include <math/seadMathCalcCommon.h>
#include <nerd/nerdMath.h>
#include "Library/Math/MathUtil.hpp"
#include "Project/Matrix/MatrixUtil.hpp"

namespace al {
    /** @brief Constructs a radar with a default frustum. */
    FrustumRadar::FrustumRadar()
        : mOrthoSide(sead::Vector3f::ex), mOrthoUp(sead::Vector3f::ey), mOrthoFront(sead::Vector3f::ez),
          mOrthoTrans(sead::Vector3f::zero), mHorizontalSlope(0.3f), mHorizontalNormFactor(1.04403f),
          mVerticalSlope(0.2f), mVerticalNormFactor(1.0098f), mStereoEyeOffset(0.0f), mStereoSlopeLeft(0.3f),
          mStereoNormFactorLeft(1.04403f), mStereoSlopeRight(0.3f), mStereoNormFactorRight(1.04403f), mNear(100.0f),
          mFar(10000.0f) {}

    /**
     * @brief Sets up the frustum from a view matrix and a perspective.
     * @param rViewMtx The camera's view matrix.
     * @param fovy The vertical field of view in degrees.
     * @param aspect The aspect ratio of the view.
     * @param near The near clip distance.
     * @param far The far clip distance.
     */
    void FrustumRadar::calcFrustumArea(const sead::Matrix34f& rViewMtx, f32 fovy, f32 aspect, f32 near, f32 far) {
        setLocalAxis(rViewMtx);
        setFactor(fovy, aspect);
        mNear = near;
        mFar = far;
    }

    /**
     * @brief Sets the frustum's axes and origin from a view matrix.
     * @param rViewMtx The camera's view matrix.
     */
    void FrustumRadar::setLocalAxis(const sead::Matrix34f& rViewMtx) {
        sead::Matrix34f invViewMtx;
        calcMxtInvertOrtho(&invViewMtx, rViewMtx);

        invViewMtx.getBase(mOrthoSide, 0);
        invViewMtx.getBase(mOrthoUp, 1);
        invViewMtx.getBase(mOrthoFront, 2);
        mOrthoFront.negate();
        invViewMtx.getTranslation(mOrthoTrans);

        mStereoEyeOffset = 0.0f;
    }

    /**
     * @brief Sets the frustum's slopes from a perspective.
     * @param fovy The vertical field of view in degrees.
     * @param aspect The aspect ratio of the view.
     */
    void FrustumRadar::setFactor(f32 fovy, f32 aspect) {
        f32 verticalSlope = sead::Mathf::tan(sead::Mathf::deg2rad(fovy * 0.5f));
        mVerticalSlope = verticalSlope;
        mVerticalNormFactor = nerd::sqrt(verticalSlope * verticalSlope + 1.0f);

        mHorizontalSlope = mVerticalSlope * aspect;
        mHorizontalNormFactor = nerd::sqrt(mHorizontalSlope * mHorizontalSlope + 1.0f);
    }

    /**
     * @brief Sets up the frustum from a view and a projection matrix.
     * @param rViewMtx The camera's view matrix.
     * @param rProjMtx The camera's projection matrix.
     * @param near The near clip distance.
     * @param far The far clip distance.
     */
    void FrustumRadar::calcFrustumArea(const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx, f32 near, f32 far) {
        setLocalAxis(rViewMtx);
        setFactor(rProjMtx);
        mNear = near;
        mFar = far;
    }

    /**
     * @brief Sets the frustum's slopes from a projection matrix.
     * @param rProjMtx The camera's projection matrix.
     */
    void FrustumRadar::setFactor(const sead::Matrix44f& rProjMtx) {
        mVerticalSlope = 1.0f / rProjMtx(1, 1);
        mVerticalNormFactor = nerd::sqrt(mVerticalSlope * mVerticalSlope + 1.0f);

        mHorizontalSlope = 1.0f / rProjMtx(0, 0);
        mHorizontalNormFactor = nerd::sqrt(mHorizontalSlope * mHorizontalSlope + 1.0f);
    }

    /**
     * @brief Sets up the frustum enclosing both eyes of a stereo view.
     * @param rViewMtxLeft The left eye's view matrix.
     * @param rViewMtxRight The right eye's view matrix.
     * @param rProjMtx The projection matrix.
     * @param near The near clip distance.
     * @param far The far clip distance.
     */
    void FrustumRadar::calcFrustumAreaStereo(const sead::Matrix34f& rViewMtxLeft, const sead::Matrix34f& rViewMtxRight,
                                             const sead::Matrix44f& rProjMtx, f32 near, f32 far) {
        setLocalAxisStereo(rViewMtxLeft, rViewMtxRight);
        setFactorStereo(rProjMtx);
        mNear = near;
        mFar = far;
    }

    /**
     * @brief Sets the frustum's axes and origin from the view matrices of both eyes.
     * @param rViewMtxLeft The left eye's view matrix.
     * @param rViewMtxRight The right eye's view matrix.
     */
    void FrustumRadar::setLocalAxisStereo(const sead::Matrix34f& rViewMtxLeft, const sead::Matrix34f& rViewMtxRight) {
        sead::Matrix34f invViewMtxLeft;
        sead::Matrix34f invViewMtxRight;
        calcMxtInvertOrtho(&invViewMtxLeft, rViewMtxLeft);

        invViewMtxLeft.getBase(mOrthoSide, 0);
        invViewMtxLeft.getBase(mOrthoUp, 1);
        invViewMtxLeft.getBase(mOrthoFront, 2);
        mOrthoFront.negate();

        calcMxtInvertOrtho(&invViewMtxRight, rViewMtxRight);
        mOrthoTrans = (invViewMtxLeft.getTranslation() + invViewMtxRight.getTranslation()) * 0.5f;

        mStereoEyeOffset = mOrthoSide.dot(mOrthoTrans - invViewMtxLeft.getTranslation());
    }

    /**
     * @brief Sets the frustum's slopes for both eyes from a projection matrix.
     * @param rProjMtx The projection matrix.
     */
    void FrustumRadar::setFactorStereo(const sead::Matrix44f& rProjMtx) {
        setFactor(rProjMtx);
        f32 centerOffset = rProjMtx(0, 2);

        mStereoSlopeLeft = mHorizontalSlope * (1.0f - centerOffset);
        mStereoNormFactorLeft = nerd::sqrt(mStereoSlopeLeft * mStereoSlopeLeft + 1.0f);

        mStereoSlopeRight = mHorizontalSlope * (1.0f + centerOffset);
        mStereoNormFactorRight = nerd::sqrt(mStereoSlopeRight * mStereoSlopeRight + 1.0f);
    }

    /**
     * @brief Checks whether a sphere is not fully outside the left plane.
     * @param rPos The center of the sphere.
     * @param radius The radius of the sphere.
     * @return Whether the sphere is inside of the left plane.
     */
    bool FrustumRadar::judgeInLeft(const sead::Vector3f& rPos, f32 radius) const {
        f32 dotFront = mOrthoFront.dot(rPos - mOrthoTrans);
        f32 dotSide = mOrthoSide.dot(rPos - mOrthoTrans);

        return !(dotSide < -(dotFront * mHorizontalSlope + mHorizontalNormFactor * radius));
    }

    /**
     * @brief Checks whether a sphere is not fully outside the right plane.
     * @param rPos The center of the sphere.
     * @param radius The radius of the sphere.
     * @return Whether the sphere is inside of the right plane.
     */
    bool FrustumRadar::judgeInRight(const sead::Vector3f& rPos, f32 radius) const {
        f32 dotFront = mOrthoFront.dot(rPos - mOrthoTrans);
        f32 dotSide = mOrthoSide.dot(rPos - mOrthoTrans);

        return !(dotFront * mHorizontalSlope + mHorizontalNormFactor * radius < dotSide);
    }

    /**
     * @brief Checks whether a sphere is not fully outside the top plane.
     * @param rPos The center of the sphere.
     * @param radius The radius of the sphere.
     * @return Whether the sphere is inside of the top plane.
     */
    bool FrustumRadar::judgeInTop(const sead::Vector3f& rPos, f32 radius) const {
        f32 dotFront = mOrthoFront.dot(rPos - mOrthoTrans);
        f32 dotUp = mOrthoUp.dot(rPos - mOrthoTrans);

        return !(dotFront * mVerticalSlope + mVerticalNormFactor * radius < dotUp);
    }

    /**
     * @brief Checks whether a sphere is not fully outside the bottom plane.
     * @param rPos The center of the sphere.
     * @param radius The radius of the sphere.
     * @return Whether the sphere is inside of the bottom plane.
     */
    bool FrustumRadar::judgeInBottom(const sead::Vector3f& rPos, f32 radius) const {
        f32 dotFront = mOrthoFront.dot(rPos - mOrthoTrans);
        f32 dotUp = mOrthoUp.dot(rPos - mOrthoTrans);

        return !(dotUp < -(dotFront * mVerticalSlope + mVerticalNormFactor * radius));
    }

    /**
     * @brief Checks whether a sphere is not fully in front of the near plane.
     * @param rPos The center of the sphere.
     * @param radius The radius of the sphere.
     * @param near The near clip distance.
     * @return Whether the sphere is behind the near plane.
     */
    bool FrustumRadar::judgeInNear(const sead::Vector3f& rPos, f32 radius, f32 near) const {
        f32 dotFront = mOrthoFront.dot(rPos - mOrthoTrans);

        return !(dotFront < near - radius);
    }

    /**
     * @brief Checks whether a sphere is not fully behind the far plane.
     * @param rPos The center of the sphere.
     * @param radius The radius of the sphere.
     * @param far The far clip distance, or a non-positive value for no far plane.
     * @return Whether the sphere is in front of the far plane.
     */
    bool FrustumRadar::judgeInFar(const sead::Vector3f& rPos, f32 radius, f32 far) const {
        f32 dotFront = mOrthoFront.dot(rPos - mOrthoTrans);

        return !(far > 0.0f && radius + far < dotFront);
    }

    /**
     * @brief Checks whether a sphere intersects the frustum.
     * @param rPos The center of the sphere.
     * @param radius The radius of the sphere.
     * @param near The near clip distance.
     * @param far The far clip distance, or a non-positive value for no far plane.
     * @return Whether the sphere is inside the frustum.
     */
    bool FrustumRadar::judgeInArea(const sead::Vector3f& rPos, f32 radius, f32 near, f32 far) const {
        f32 dotFront = mOrthoFront.dot(rPos - mOrthoTrans);

        if (dotFront < near - radius) {
            return false;
        }

        if (far > 0.0f && radius + far < dotFront) {
            return false;
        }

        f32 dotUpAbs = sead::Mathf::abs(mOrthoUp.dot(rPos - mOrthoTrans));
        if (dotFront * mVerticalSlope + mVerticalNormFactor * radius < dotUpAbs) {
            return false;
        }

        f32 dotSide = mOrthoSide.dot(rPos - mOrthoTrans);
        if (isNearZero(mStereoEyeOffset, 0.001f)) {
            if (dotFront * mHorizontalSlope + mHorizontalNormFactor * radius < sead::Mathf::abs(dotSide)) {
                return false;
            }
        } else {
            f32 limitRight = dotFront * mStereoSlopeRight + mStereoNormFactorRight * radius;
            f32 limitLeft = dotFront * mStereoSlopeLeft + mStereoNormFactorLeft * radius;

            f32 relSideLeft = dotSide - mStereoEyeOffset;
            f32 relSideRight = dotSide + mStereoEyeOffset;

            if (relSideLeft > limitLeft && relSideRight > limitRight) {
                return false;
            }

            if (relSideLeft < -limitRight && relSideRight < -limitLeft) {
                return false;
            }
        }

        return true;
    }

    /**
     * @brief Checks whether a set of points could span a volume intersecting the frustum, i.e. whether
     * the points are not all outside of the same plane.
     * @param pPoints The points to check.
     * @param numPoints The number of points.
     * @param near The near clip distance.
     * @param far The far clip distance, or a negative value to use the radar's far clip distance.
     * @return Whether the points are not all outside of one of the frustum's planes.
     */
    bool FrustumRadar::judgeInArea(const sead::Vector3f* pPoints, s32 numPoints, f32 near, f32 far) const {
        if (far < 0.0f) {
            far = mFar;
        }

        bool isIn = false;
        for (s32 i = 0; i < numPoints; i++) {
            if (judgeInLeft(pPoints[i], 0.0f)) {
                isIn = true;
                break;
            }
        }
        if (!isIn) {
            return false;
        }

        isIn = false;
        for (s32 i = 0; i < numPoints; i++) {
            if (judgeInRight(pPoints[i], 0.0f)) {
                isIn = true;
                break;
            }
        }
        if (!isIn) {
            return false;
        }

        isIn = false;
        for (s32 i = 0; i < numPoints; i++) {
            if (judgeInTop(pPoints[i], 0.0f)) {
                isIn = true;
                break;
            }
        }
        if (!isIn) {
            return false;
        }

        isIn = false;
        for (s32 i = 0; i < numPoints; i++) {
            if (judgeInBottom(pPoints[i], 0.0f)) {
                isIn = true;
                break;
            }
        }
        if (!isIn) {
            return false;
        }

        isIn = false;
        for (s32 i = 0; i < numPoints; i++) {
            if (judgeInNear(pPoints[i], 0.0f, near)) {
                isIn = true;
                break;
            }
        }
        if (!isIn) {
            return false;
        }

        isIn = false;
        for (s32 i = 0; i < numPoints; i++) {
            if (judgeInFar(pPoints[i], 0.0f, far)) {
                isIn = true;
                break;
            }
        }
        if (!isIn) {
            return false;
        }

        return true;
    }

    /**
     * @brief Checks whether a sphere intersects the frustum, using the radar's far clip distance.
     * @param rPos The center of the sphere.
     * @param radius The radius of the sphere.
     * @param near The near clip distance.
     * @return Whether the sphere is inside the frustum.
     */
    bool FrustumRadar::judgeInArea(const sead::Vector3f& rPos, f32 radius, f32 near) const {
        return judgeInArea(rPos, radius, near, mFar);
    }

    /**
     * @brief Checks whether a sphere intersects the frustum, using the radar's clip distances.
     * @param rPos The center of the sphere.
     * @param radius The radius of the sphere.
     * @return Whether the sphere is inside the frustum.
     */
    bool FrustumRadar::judgeInArea(const sead::Vector3f& rPos, f32 radius) const {
        return judgeInArea(rPos, radius, mNear, mFar);
    }

    /**
     * @brief Checks whether a sphere intersects the frustum without a far plane.
     * @param rPos The center of the sphere.
     * @param radius The radius of the sphere.
     * @return Whether the sphere is inside the frustum.
     */
    bool FrustumRadar::judgeInAreaNoFar(const sead::Vector3f& rPos, f32 radius) const {
        return judgeInArea(rPos, radius, mNear, -1.0f);
    }
};

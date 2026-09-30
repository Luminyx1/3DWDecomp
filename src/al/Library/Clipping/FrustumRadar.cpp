#include "Library/Clipping/FrustumRadar.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/Math/MathUtil.hpp"
#include "Library/Math/MatrixUtil.hpp"

namespace al {

FrustumRadar::FrustumRadar() = default;

void FrustumRadar::calcFrustumArea(const sead::Matrix34f& rOrthoMtx, f32 fovyAngle, f32 aspectRatio,
                                   f32 near, f32 far) {
    setLocalAxis(rOrthoMtx);

    setFactor(fovyAngle, aspectRatio);

    mNear = near;
    mFar = far;
}

void FrustumRadar::setLocalAxis(const sead::Matrix34f& rOrthoMtx) {
    sead::Matrix34f mtxInvertOrtho;
    calcMxtInvertOrtho(&mtxInvertOrtho, rOrthoMtx);

    mtxInvertOrtho.getBase(mOrthoSide, 0);
    mtxInvertOrtho.getBase(mOrthoUp, 1);
    mtxInvertOrtho.getBase(mOrthoFront, 2);
    mOrthoFront.negate();
    mtxInvertOrtho.getTranslation(mOrthoTrans);

    mStereoEyeOffset = 0.0f;
}

void FrustumRadar::setFactor(f32 fovyAngle, f32 aspectRatio) {
    f32 verticalSlope = sead::Mathf::tan(sead::Mathf::deg2rad(fovyAngle * 0.5f));
    mVerticalSlope = verticalSlope;
    mVerticalNormFactor = sead::Mathf::sqrt(verticalSlope * verticalSlope + 1.0f);

    mHorizontalSlope = mVerticalSlope * aspectRatio;
    mHorizontalNormFactor = sead::Mathf::sqrt(mHorizontalSlope * mHorizontalSlope + 1.0f);
}

void FrustumRadar::calcFrustumArea(const sead::Matrix34f& rOrthoMtx,
                                   const sead::Matrix44f& rProjectionMtx, f32 near, f32 far) {
    setLocalAxis(rOrthoMtx);
    setFactor(rProjectionMtx);
    mNear = near;
    mFar = far;
}

void FrustumRadar::setFactor(const sead::Matrix44f& rProjectionMtx) {
    mVerticalSlope = 1.0f / rProjectionMtx(1, 1);
    mVerticalNormFactor = sead::Mathf::sqrt(mVerticalSlope * mVerticalSlope + 1.0f);

    mHorizontalSlope = 1.0f / rProjectionMtx(0, 0);
    mHorizontalNormFactor = sead::Mathf::sqrt(mHorizontalSlope * mHorizontalSlope + 1.0f);
}

void FrustumRadar::calcFrustumAreaStereo(const sead::Matrix34f& rOrthoMtxLeft,
                                         const sead::Matrix34f& rOrthoMtxRight,
                                         const sead::Matrix44f& rProjectionMtx, f32 near, f32 far) {
    setLocalAxisStereo(rOrthoMtxLeft, rOrthoMtxRight);
    setFactorStereo(rProjectionMtx);
    mNear = near;
    mFar = far;
}

void FrustumRadar::setLocalAxisStereo(const sead::Matrix34f& rOrthoMtxLeft,
                                      const sead::Matrix34f& rOrthoMtxRight) {
    sead::Matrix34f mtxInvertOrthoLeft;
    sead::Matrix34f mtxInvertRight;
    calcMxtInvertOrtho(&mtxInvertOrthoLeft, rOrthoMtxLeft);

    mtxInvertOrthoLeft.getBase(mOrthoSide, 0);
    mtxInvertOrthoLeft.getBase(mOrthoUp, 1);
    mtxInvertOrthoLeft.getBase(mOrthoFront, 2);
    mOrthoFront.negate();

    calcMxtInvertOrtho(&mtxInvertRight, rOrthoMtxRight);
    mOrthoTrans = (mtxInvertOrthoLeft.getTranslation() + mtxInvertRight.getTranslation()) * 0.5f;

    mStereoEyeOffset = mOrthoSide.dot(mOrthoTrans - mtxInvertOrthoLeft.getTranslation());
}

void FrustumRadar::setFactorStereo(const sead::Matrix44f& rProjectionMtx) {
    setFactor(rProjectionMtx);
    f32 centerOffset = rProjectionMtx(0, 2);

    mStereoSlopeLeft = mHorizontalSlope * (1.0f - centerOffset);
    mStereoNormFactorLeft = sead::Mathf::sqrt(mStereoSlopeLeft * mStereoSlopeLeft + 1.0f);

    mStereoSlopeRight = mHorizontalSlope * (1.0f + centerOffset);
    mStereoNormFactorRight = sead::Mathf::sqrt(mStereoSlopeRight * mStereoSlopeRight + 1.0f);
}

bool FrustumRadar::judgeInLeft(const sead::Vector3f& rPos, f32 radius) const {
    f32 dotFront = mOrthoFront.dot(rPos - mOrthoTrans);
    f32 dotSide = mOrthoSide.dot(rPos - mOrthoTrans);

    return !(dotSide < -(dotFront * mHorizontalSlope + mHorizontalNormFactor * radius));
}

bool FrustumRadar::judgeInRight(const sead::Vector3f& rPos, f32 radius) const {
    f32 dotFront = mOrthoFront.dot(rPos - mOrthoTrans);
    f32 dotSide = mOrthoSide.dot(rPos - mOrthoTrans);

    return !(dotFront * mHorizontalSlope + mHorizontalNormFactor * radius < dotSide);
}

bool FrustumRadar::judgeInTop(const sead::Vector3f& rPos, f32 radius) const {
    f32 dotFront = mOrthoFront.dot(rPos - mOrthoTrans);
    f32 dotUp = mOrthoUp.dot(rPos - mOrthoTrans);

    return !(dotFront * mVerticalSlope + mVerticalNormFactor * radius < dotUp);
}

bool FrustumRadar::judgeInBottom(const sead::Vector3f& rPos, f32 radius) const {
    f32 dotFront = mOrthoFront.dot(rPos - mOrthoTrans);
    f32 dotUp = mOrthoUp.dot(rPos - mOrthoTrans);

    return !(dotUp < -(dotFront * mVerticalSlope + mVerticalNormFactor * radius));
}

bool FrustumRadar::judgeInNear(const sead::Vector3f& rPos, f32 radius, f32 near) const {
    f32 dotFront = mOrthoFront.dot(rPos - mOrthoTrans);
    return !(dotFront < near - radius);
}

bool FrustumRadar::judgeInFar(const sead::Vector3f& rPos, f32 radius, f32 far) const {
    f32 dotFront = mOrthoFront.dot(rPos - mOrthoTrans);
    return !(far > 0.0f && radius + far < dotFront);
}

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
    if (isNearZero(mStereoEyeOffset)) {
        if (dotFront * mHorizontalSlope + mHorizontalNormFactor * radius <
            sead::Mathf::abs(dotSide)) {
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

bool FrustumRadar::judgeInArea(const sead::Vector3f* pPoints, s32 numPoints, f32 near,
                               f32 far) const {
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
    return isIn;
}

bool FrustumRadar::judgeInArea(const sead::Vector3f& rPos, f32 radius, f32 near) const {
    return judgeInArea(rPos, radius, near, mFar);
}

bool FrustumRadar::judgeInArea(const sead::Vector3f& rPos, f32 radius) const {
    return judgeInArea(rPos, radius, mNear, mFar);
}

bool FrustumRadar::judgeInAreaNoFar(const sead::Vector3f& rPos, f32 radius) const {
    return judgeInArea(rPos, radius, mNear, -1.0f);
}
}  // namespace al

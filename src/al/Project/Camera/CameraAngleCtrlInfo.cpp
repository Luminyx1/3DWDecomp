#include "Project/Camera/CameraAngleCtrlInfo.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/Camera/CameraPoserFunction.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ByamlUtil.hpp"

namespace al {

CameraAngleCtrlInfo::CameraAngleCtrlInfo() {
    mResetInfo = new ResetInfo();
    mRequestInfo = new RequestInfo();
}

/**
 * Creates angle control info whose horizontal angle is relative to the start pose.
 * @return New angle control info.
 */
CameraAngleCtrlInfo* CameraAngleCtrlInfo::createWithRelativeH() {
    CameraAngleCtrlInfo* info = new CameraAngleCtrlInfo();
    info->mRotateHType = 1;
    info->mIsValidRotateH = true;
    return info;
}

/**
 * Loads the angle parameters from the "Angle" block, or from the root when it is missing.
 * @param rIter Camera parameter iterator.
 */
void CameraAngleCtrlInfo::load(const ByamlIter& rIter) {
    ByamlIter angleIter;
    ByamlIter iter(rIter.tryGetIterByKey(&angleIter, "Angle") ? angleIter : rIter);
    if (tryGetByamlBool(&mIsValidRotateH, iter, "IsValidRotateH") && mIsValidRotateH) {
        tryGetByamlF32(&mMinAngleH, iter, "MinAngleH");
        tryGetByamlF32(&mMaxAngleH, iter, "MaxAngleH");
    }
    tryGetByamlF32(&mStartAngleV, iter, "AngleV");
    tryGetByamlF32(&mDefaultMinAngleV, iter, "MinAngleV");
    tryGetByamlF32(&mDefaultMaxAngleV, iter, "MaxAngleV");
    mIsKeepPreAngleV = tryGetByamlKeyBoolOrFalse(iter, "IsKeepPreAngleV");
    if (tryGetByamlF32(&mResetAngleV, iter, "ResetAngleV")) {
        mIsSetResetAngleV = true;
    }
    tryGetByamlBool(&mIsInvalidReceiveRequest, iter, "IsInvalidReceiveRequest");
}

/**
 * Resets the angles when the camera starts.
 * @param angleV Vertical angle of the previous camera, used when the previous angle is kept.
 */
void CameraAngleCtrlInfo::start(f32 angleV) {
    if (mRotateHType == 1) {
        mAngleH = 0.0f;
        mTargetAngleH = 0.0f;
    }
    f32 startAngleV = mIsKeepPreAngleV ? angleV : mStartAngleV;
    f32 clampedAngleV = sead::Mathf::clamp(startAngleV, mDefaultMinAngleV, mDefaultMaxAngleV);
    mAngleV = clampedAngleV;
    mTargetAngleV = clampedAngleV;
    mResetInfo->step = -1;
    mRequestInfo->step = -1;
}

void CameraAngleCtrlInfo::update(const sead::Vector2f& rStick, f32 sensitivityScale,
                                 bool isTriggerReset) {
    mSensitivityScale = sensitivityScale;

    if (mResetInfo->step >= 0) {
        bool isNoInput = isNearZero(rStick, 0.001f);
        ResetInfo* info = mResetInfo;
        if (isNoInput) {
            if (info->step >= 0) {
                info->step = info->maxStep <= info->step + 1 ? -1 : info->step + 1;
            }
        } else {
            info->step = -1;
        }
    }

    if (isTriggerReset) {
        f32 resetAngleV = mIsSetResetAngleV ? mResetAngleV : mStartAngleV;
        f32 angleH = mAngleH;
        f32 angleV = mAngleV;
        f32 diffV = angleV - resetAngleV;
        f32 diff = sead::Mathf::square(angleH) < sead::Mathf::square(diffV) ? diffV : angleH;
        ResetInfo* info = mResetInfo;
        f32 absDiff = sign(diff) * diff;
        if (!(absDiff < 5.0f)) {
            info->startAngleH = angleH;
            info->startAngleV = angleV;
            info->step = 0;
            info->maxStep = sead::Mathi::max((s32)(absDiff / 5.0f), 15);
            info->targetAngleV = resetAngleV;
        }
        if (mRequestInfo->step >= 0) {
            mRequestInfo->step = -1;
        }
    }

    if (mRequestInfo->step >= 0) {
        bool isNoInput = isNearZero(rStick.y, 0.001f);
        RequestInfo* info = mRequestInfo;
        s32 step = -1;
        if (isNoInput) {
            step = info->maxStep <= info->step + 1 ? -1 : info->step + 1;
        }
        info->step = step;
    }

    if (mRotateHType == 1) {
        if (!mIsValidRotateH) {
            mAngleH = 0.0f;
            mTargetAngleH = 0.0f;
        } else if (mResetInfo->step >= 0) {
            ResetInfo* info = mResetInfo;
            f32 startAngleH = info->startAngleH;
            f32 rate = squareOut(normalize((f32)info->step, 0.0f, (f32)info->maxStep));
            f32 angleH = startAngleH - rate * info->startAngleH;
            mAngleH = angleH;
            mTargetAngleH = angleH;
        } else {
            f32 threshold = mStickThreshold;
            f32 targetAngleH = mTargetAngleH;
            f32 moveH =
                isNearZero(rStick.x, threshold) ? 0.0f : normalizeAbs(rStick.x, threshold, 1.0f) * 2.7f;
            f32 nextAngleH = sead::Mathf::clamp(targetAngleH + moveH * sensitivityScale,
                                                mMinAngleH, mMaxAngleH);
            mTargetAngleH = lerpValue(0.6f, targetAngleH, nextAngleH);
            mAngleH = lerpValue(0.2f, mAngleH, mTargetAngleH);
        }
    }

    if (mResetInfo->step >= 0) {
        ResetInfo* info = mResetInfo;
        f32 rate = squareOut(normalize((f32)info->step, 0.0f, (f32)info->maxStep));
        mTargetAngleV = lerpValue(rate, info->startAngleV, info->targetAngleV);
        mAngleV = mTargetAngleV;
    } else if (mRequestInfo->step >= 0) {
        RequestInfo* info = mRequestInfo;
        f32 startAngleV = info->startAngleV;
        f32 targetAngleV = info->targetAngleV;
        f32 rate = easeInOut(normalize((f32)info->step, 0.0f, (f32)info->maxStep));
        mTargetAngleV = lerpDegree(startAngleV, targetAngleV, rate);
        mAngleV = mTargetAngleV;
    } else {
        f32 threshold = mStickThreshold;
        f32 speedV = mSpeedV;
        f32 targetLerpRate = mTargetLerpRateV;
        f32 targetAngleV = mTargetAngleV;
        f32 moveV = 0.0f;
        if (!isNearZero(rStick.y, threshold)) {
            moveV = -(speedV * normalize(sead::Mathf::abs(rStick.y), threshold, 1.0f)) *
                    sign(rStick.y);
        }
        f32 nextAngleV =
            lerpValue(targetLerpRate, targetAngleV, targetAngleV + moveV * sensitivityScale);
        mTargetAngleV = sead::Mathf::clamp(nextAngleV, mDefaultMinAngleV, mDefaultMaxAngleV);
        mAngleV = lerpValue(mLerpRateV, mAngleV, mTargetAngleV);
    }
}

/**
 * Starts moving to a vertical angle requested by an object.
 * @param rInfo Object request.
 * @return Whether the request was accepted.
 */
bool CameraAngleCtrlInfo::receiveRequestFromObject(const CameraObjectRequestInfo& rInfo) {
    if (mIsInvalidReceiveRequest) {
        return false;
    }
    if (!alCameraPoserFunction::isRequestSetAngleV(rInfo)) {
        return false;
    }
    if (mResetInfo->step >= 0) {
        return false;
    }
    f32 requestAngleV = alCameraPoserFunction::getRequestAngleV(rInfo);
    f32 diff = sead::Mathf::abs(diffNearAngleDegree(mAngleV, requestAngleV));
    f32 speed = mSensitivityScale * 0.5f;
    if (speed <= 0.0f || diff < speed) {
        return false;
    }
    RequestInfo* info = mRequestInfo;
    info->startAngleV = mAngleV;
    info->targetAngleV = requestAngleV;
    info->step = 0;
    info->maxStep = diff / speed;
    return true;
}

/**
 * Sets the vertical angle, clamped to the vertical range.
 * @param angleV Vertical angle in degrees.
 */
void CameraAngleCtrlInfo::setAngleV(f32 angleV) {
    f32 clampedAngleV = angleV;
    if (mDefaultMinAngleV > angleV) {
        clampedAngleV = mDefaultMinAngleV;
    } else if (mDefaultMaxAngleV < angleV) {
        clampedAngleV = mDefaultMaxAngleV;
    }
    mAngleV = clampedAngleV;
    mTargetAngleV = clampedAngleV;
}

/**
 * Checks whether the angle ranges leave no room to rotate.
 * @return Whether both ranges are fixed.
 */
bool CameraAngleCtrlInfo::isFixByRangeHV() const {
    if (mIsValidRotateH && !isNear(mMinAngleH, mMaxAngleH, 0.001f)) {
        return false;
    }
    return isNear(mDefaultMinAngleV, mDefaultMaxAngleV, 0.001f);
}

/**
 * Checks whether a reset started this frame.
 * @return Whether the reset is on its first step.
 */
bool CameraAngleCtrlInfo::isResetStartTiming() const {
    return mResetInfo->step == 0;
}

/**
 * Gets the length of the current reset.
 * @return Number of reset steps.
 */
s32 CameraAngleCtrlInfo::getMaxResetStep() const {
    return mResetInfo->maxStep;
}

}  // namespace al

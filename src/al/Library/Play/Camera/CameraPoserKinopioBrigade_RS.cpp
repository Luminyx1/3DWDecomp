#include "Library/Play/Camera/CameraPoserKinopioBrigade_RS.hpp"

#include <cmath>
#include <gfx/seadCamera.h>
#include <math/seadMathCalcCommon.h>
#include <math/seadMatrix.h>
#include <math/seadQuat.h>

#include "Library/Camera/CameraPoserFunction.hpp"
#include "Library/Controller/InputFunction.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ByamlUtil.hpp"

namespace {

/**
 * Calculates the pitch of the pad from its pose.
 * @param rAxisY Y axis of the pad pose.
 * @param rAxisZ Z axis of the pad pose.
 * @return Pitch in degrees.
 */
f32 calcPadAngleV(const sead::Vector3f& rAxisY, const sead::Vector3f& rAxisZ) {
    f32 lengthH = sead::Mathf::sqrt(rAxisY.x * rAxisY.x + rAxisY.z * rAxisY.z);

    if (rAxisZ.y <= 0.0f) {
        lengthH = -lengthH;
    }

    return sead::Mathf::rad2deg(std::atan2(rAxisY.y, lengthH));
}

/**
 * Calculates the yaw of the pad from its pose.
 * @param rAxisX X axis of the pad pose.
 * @return Yaw in degrees.
 */
f32 calcPadAngleH(const sead::Vector3f& rAxisX) {
    return sead::Mathf::rad2deg(std::atan2(-rAxisX.z, rAxisX.x));
}

/**
 * Calculates the horizontal angle of a direction, measured from the Z axis.
 * @param pAngleH Output for the angle in degrees.
 * @param rDir Direction.
 * @return Whether the direction has a horizontal component.
 */
bool tryCalcAngleH(f32* pAngleH, const sead::Vector3f& rDir) {
    sead::Vector3f dirH = {rDir.x, 0.0f, rDir.z};

    if (!al::tryNormalizeOrZero(&dirH)) {
        return false;
    }

    f32 cos = sead::Mathf::clamp(dirH.dot(sead::Vector3f::ez), -1.0f, 1.0f);
    *pAngleH = sead::Mathf::rad2deg(std::acos(cos));

    if (dirH.x < 0.0f) {
        *pAngleH = -*pAngleH;
    }

    return true;
}

}  // namespace

namespace al {

/**
 * Creates the camera of the Captain Toad levels, controlled by the stick and the pad gyro.
 * @param pName Camera name.
 */
CameraPoserKinopioBrigade_RS::CameraPoserKinopioBrigade_RS(const char* pName)
    : CameraPoser_RS(pName) {}

/**
 * Starts at the start angles, or at the angles of the previous camera.
 * @param rInfo Start info.
 */
void CameraPoserKinopioBrigade_RS::start(const CameraStartInfo& rInfo) {
    if (mIsResetForce || (mIsSetStartAngle && isFirstCalc())) {
        resetValues(mStartAngleH, mStartAngleV);
    } else {
        sead::Vector3f dir = alCameraPoserFunction::getPreCameraPos(this) -
                             alCameraPoserFunction::getPreLookAtPos(this);

        if (!tryNormalizeOrZero(&dir)) {
            return;
        }

        f32 angleV;

        if (tryCalcAngleH(&mAngleH, dir)) {
            angleV = sead::Mathf::rad2deg(std::asin(sead::Mathf::clamp(dir.y, -1.0f, 1.0f)));
        } else {
            mAngleH = 0.0f;
            angleV = !(dir.y >= 0.0f) ? 90.0f : -90.0f;
            mAngleV = angleV;
        }

        resetValues(mAngleH, angleV);
    }

    update();
}

/**
 * Resets the camera to the given angles relative to the current pad pose.
 * @param angleH Horizontal angle in degrees.
 * @param angleV Vertical angle in degrees.
 */
void CameraPoserKinopioBrigade_RS::resetValues(f32 angleH, f32 angleV) {
    sead::Vector3f axisX;
    sead::Vector3f axisY;
    sead::Vector3f axisZ;
    getPadPose(&axisX, &axisY, &axisZ, getMainControllerPort(), 0);
    f32 padAngleV = calcPadAngleV(axisY, axisZ);
    f32 padAngleH = calcPadAngleH(axisX);
    mSmoothDrcAngleH = padAngleH;
    mDrcAngleH = padAngleH;
    mPrevPadAngleH = padAngleH;
    mSmoothDrcAngleV = padAngleV;
    mPrevPadAngleV = padAngleV;
    mDrcAngleV = padAngleV;
    mBaseAngleV = angleV - padAngleV;
    mCurrentBaseAngleV = angleV - padAngleV;
    mBaseAngleH = angleH - padAngleH;
    mCurrentBaseAngleH = angleH - padAngleH;
}

/**
 * Updates the angles from the pad and the stick and places the camera.
 */
void CameraPoserKinopioBrigade_RS::update() {
    calcDrcAngle();
    calcBaseAngle();
    convergeBaseAngle();
    mAngleV = sead::Mathf::clamp(mSmoothDrcAngleV + mCurrentBaseAngleV, mAngleVMin, mAngleVMax);
    mAngleH = wrapAngle(mSmoothDrcAngleH + mCurrentBaseAngleH);
    commitCamera();
}

/**
 * Follows the rotation of the pad.
 */
void CameraPoserKinopioBrigade_RS::calcDrcAngle() {
    sead::Vector3f axisX;
    sead::Vector3f axisY;
    sead::Vector3f axisZ;
    getPadPose(&axisX, &axisY, &axisZ, getMainControllerPort(), 0);

    if (isNearAngleDegree(axisX, mPadAxisX, 5.0f) && isNearAngleDegree(axisY, mPadAxisY, 5.0f) &&
        isNearAngleDegree(axisZ, mPadAxisZ, 5.0f)) {
        axisX.set(mPadAxisX);
        axisY.set(mPadAxisY);
        axisZ.set(mPadAxisZ);
    } else {
        mPadAxisX.set(axisX);
        mPadAxisY.set(axisY);
        mPadAxisZ.set(axisZ);
    }

    f32 padAngleV = calcPadAngleV(axisY, axisZ);
    f32 padAngleH = calcPadAngleH(axisX);
    mDrcAngleH = padAngleH;

    f32 drcAngleV = mDrcAngleV + mBaseAngleV;
    f32 diff = padAngleV - mPrevPadAngleV;
    f32 diffLess = padAngleV + -360.0f - mPrevPadAngleV;
    f32 diffMore = padAngleV - (mPrevPadAngleV + -360.0f);
    f32 absDiff = sead::Mathf::abs(diff);
    f32 absDiffLess = sead::Mathf::abs(diffLess);
    f32 absDiffMore = sead::Mathf::abs(diffMore);
    s32 index;

    if (absDiff < absDiffLess) {
        index = absDiff < absDiffMore ? 0 : 2;
    } else {
        index = absDiffLess < absDiffMore ? 1 : 2;
    }

    switch (index) {
    case 1:
        diff = diffLess;
        break;
    case 2:
        diff = diffMore;
        break;
    }

    f32 angleV = drcAngleV + diff;

    if (isInRange(angleV, mAngleVMin, mAngleVMax)) {
        mDrcAngleV = diff + mDrcAngleV;
    } else if (angleV < mAngleVMin) {
        mDrcAngleV = mAngleVMin - mBaseAngleV;
    } else if (mAngleVMax < angleV) {
        mDrcAngleV = mAngleVMax - mBaseAngleV;
    }

    mPrevPadAngleV = padAngleV;
    mPrevPadAngleH = padAngleH;
    mSmoothDrcAngleV = lerpValue(mDrcAngleV, 0.1f, mSmoothDrcAngleV);
    mSmoothDrcAngleH = lerpDegree(mSmoothDrcAngleH, mDrcAngleH, 0.1f);
}

/**
 * Rotates the camera with the stick or resets it to the start angles.
 */
void CameraPoserKinopioBrigade_RS::calcBaseAngle() {
    getMainControllerPort();

    if (mResetFrame == 0) {
        sead::Vector2f stick;
        alCameraPoserFunction::calcCameraRotateStick(&stick, this);

        if (sead::Mathf::abs(stick.x) > 0.3f || sead::Mathf::abs(stick.y) > 0.3f) {
            f32 scale = alCameraPoserFunction::getStickSensitivityScale(this) * -1.5f;

            if (sead::Mathf::abs(stick.x) < sead::Mathf::abs(stick.y)) {
                f32 currentAngleV = mDrcAngleV + mBaseAngleV;
                f32 diff = scale * stick.y;
                f32 angleV = diff + currentAngleV;

                if (isInRange(angleV, mAngleVMin, mAngleVMax)) {
                    mBaseAngleV = diff + mBaseAngleV;
                } else if (angleV < mAngleVMin) {
                    mBaseAngleV = mAngleVMin - mDrcAngleV;
                } else if (mAngleVMax < angleV) {
                    mBaseAngleV = mAngleVMax - mDrcAngleV;
                }
            } else {
                mBaseAngleH = stick.x * scale + mBaseAngleH;
            }
        }
    }

    if (!alCameraPoserFunction::isTriggerCameraResetRotate(this) || mResetFrame != 0) {
        return;
    }

    mBaseAngleV = mStartAngleV - mDrcAngleV;
    mBaseAngleH = mStartAngleH - mDrcAngleH;
    mResetSpeedV =
        sead::Mathf::clampMin(sead::Mathf::abs(mBaseAngleV - mCurrentBaseAngleV) / 30.0f, 3.0f);
    mResetFrame = 30;
    mResetSpeedH = sead::Mathf::clampMin(
        sead::Mathf::abs(diffNearAngleDegree(mBaseAngleH, mCurrentBaseAngleH)) / mResetFrame,
        3.0f);
}

/**
 * Moves the current base angles towards the target base angles.
 */
void CameraPoserKinopioBrigade_RS::convergeBaseAngle() {
    if (mResetFrame >= 1) {
        mCurrentBaseAngleV = converge(mCurrentBaseAngleV, mBaseAngleV, mResetSpeedV);
        mBaseAngleH = wrapAngle(mBaseAngleH);
        mCurrentBaseAngleH = convergeDegree(mCurrentBaseAngleH, mBaseAngleH, mResetSpeedH);
        mResetFrame--;

        if (isNear(mBaseAngleH, mCurrentBaseAngleH, 0.001f) &&
            isNear(mBaseAngleV, mCurrentBaseAngleV, 0.001f)) {
            mResetFrame = 0;
        }
    } else {
        mCurrentBaseAngleV = converge(mCurrentBaseAngleV, mBaseAngleV, 3.0f);
        mBaseAngleH = wrapAngle(mBaseAngleH);
        mCurrentBaseAngleH = convergeDegree(mCurrentBaseAngleH, mBaseAngleH, 3.0f);
    }
}

/**
 * Places the camera around the look at position at the current angles.
 */
void CameraPoserKinopioBrigade_RS::commitCamera() {
    sead::Vector3f targetTrans = {0.0f, 0.0f, 0.0f};
    alCameraPoserFunction::calcTargetTrans(&targetTrans, this);
    followLookAt(0.1f, targetTrans, mFollowLookAtPlayerOffset, mIsFollowLookAtPlayer);
    mUp.set(sead::Vector3f::ey);

    sead::Vector3f front = sead::Vector3f::ez;

    sead::Quatf quatV;
    quatV.setAxisAngle(-sead::Vector3f::ex, mAngleV);
    sead::Matrix34f mtxV;
    mtxV.fromQuat(quatV);

    sead::Quatf quatH;
    quatH.setAxisAngle(sead::Vector3f::ey, mAngleH);
    sead::Matrix34f mtxH;
    mtxH.fromQuat(quatH);

    sead::Matrix34f mtx;
    mtx.setMul(mtxH, mtxV);
    front.mul(mtx);

    f32 distance;

    if (mIsSetDistanceDetail) {
        f32 distanceY = mDistanceY;
        f32 radianH = sead::Mathf::deg2rad(mAngleH);
        f32 distanceX = mDistanceX;
        f32 distanceZ = mDistanceZ;
        f32 angleV = mAngleV;
        distanceX *= std::cos(radianH);
        distanceZ *= std::sin(radianH);
        f32 distanceH = sead::Mathf::sqrt(distanceX * distanceX + distanceZ * distanceZ);
        f32 radianV = sead::Mathf::deg2rad(angleV);
        f32 distanceXZ = distanceH * std::cos(radianV);
        f32 distanceV = distanceY * std::sin(radianV);
        distance = sead::Mathf::sqrt(distanceXZ * distanceXZ + distanceV * distanceV);
    } else {
        distance = mDistance;
    }

    mEye = mAt + front * distance;
}

/**
 * Moves the look at position towards the target, inside the move limits.
 * @param rate Rate the look at position follows the target with.
 * @param rTargetTrans Position of the target.
 * @param rOffset Offset from the target.
 * @param rIsFollow Axes the look at position follows the target on.
 */
void CameraPoserKinopioBrigade_RS::followLookAt(f32 rate, const sead::Vector3f& rTargetTrans,
                                           const sead::Vector3f& rOffset,
                                           const V3bool& rIsFollow) {
    sead::Vector3f pos = rTargetTrans + rOffset;

    if (mIsUseMoveLimitMin.x && pos.x < mMoveLimitMin.x) {
        pos.x = mMoveLimitMin.x;
    }

    if (mIsUseMoveLimitMax.x && pos.x > mMoveLimitMax.x) {
        pos.x = mMoveLimitMax.x;
    }

    if (mIsUseMoveLimitMin.y && pos.y < mMoveLimitMin.y) {
        pos.y = mMoveLimitMin.y;
    }

    if (mIsUseMoveLimitMax.y && pos.y > mMoveLimitMax.y) {
        pos.y = mMoveLimitMax.y;
    }

    if (mIsUseMoveLimitMin.z && pos.z < mMoveLimitMin.z) {
        pos.z = mMoveLimitMin.z;
    }

    if (mIsUseMoveLimitMax.z && pos.z > mMoveLimitMax.z) {
        pos.z = mMoveLimitMax.z;
    }

    if (rIsFollow.x) {
        mAt.x = (1.0f - rate) * mAt.x + pos.x * rate;
    }

    if (rIsFollow.y) {
        mAt.y = (1.0f - rate) * mAt.y + pos.y * rate;
    }

    if (rIsFollow.z) {
        mAt.z = (1.0f - rate) * mAt.z + pos.z * rate;
    }
}

/**
 * Loads the look at position, the distances, the angles and the move limits.
 * @param rIter Camera parameter iterator.
 */
void CameraPoserKinopioBrigade_RS::loadParam(const ByamlIter& rIter) {
    mIsFollowLookAtPlayer.x = tryGetByamlKeyBoolOrFalse(rIter, "IsFollowLookAtPlayerX");
    mIsFollowLookAtPlayer.y = tryGetByamlKeyBoolOrFalse(rIter, "IsFollowLookAtPlayerY");
    mIsFollowLookAtPlayer.z = tryGetByamlKeyBoolOrFalse(rIter, "IsFollowLookAtPlayerZ");

    if (!mIsFollowLookAtPlayer.x || !mIsFollowLookAtPlayer.y || !mIsFollowLookAtPlayer.z) {
        sead::Vector3f lookAtPos = {0.0f, 0.0f, 0.0f};

        if (tryGetByamlV3f(&lookAtPos, rIter, "LookAtPos")) {
            lookAtPos.mul(getViewMtx());

            if (!mIsFollowLookAtPlayer.x) {
                mAt.x = lookAtPos.x;
            }

            if (!mIsFollowLookAtPlayer.y) {
                mAt.y = lookAtPos.y;
            }

            if (!mIsFollowLookAtPlayer.z) {
                mAt.z = lookAtPos.z;
            }
        }
    }

    tryGetByamlF32(&mDistance, rIter, "Distance");
    mIsSetDistanceDetail = tryGetByamlKeyBoolOrFalse(rIter, "IsSetDistanceDetail");
    tryGetByamlF32(&mDistanceX, rIter, "DistanceX");
    tryGetByamlF32(&mDistanceY, rIter, "DistanceY");
    tryGetByamlF32(&mDistanceZ, rIter, "DistanceZ");

    ByamlIter angleVIter;

    if (rIter.tryGetIterByKey(&angleVIter, "AngleV")) {
        tryGetByamlF32(&mAngleVMin, angleVIter, "Min");
        tryGetByamlF32(&mAngleVMax, angleVIter, "Max");
    }

    mIsSetStartAngle = tryGetByamlKeyBoolOrFalse(rIter, "IsSetStartAngle");

    if (mIsSetStartAngle) {
        ByamlIter angleStartIter;

        if (rIter.tryGetIterByKey(&angleStartIter, "AngleStart")) {
            tryGetByamlF32(&mStartAngleV, angleStartIter, "Vertical");
            tryGetByamlF32(&mStartAngleH, angleStartIter, "Horizontal");
        }

        mIsResetForce = tryGetByamlKeyBoolOrFalse(rIter, "IsResetForce");
    } else {
        mIsResetForce = false;
    }

    tryGetByamlV3f(&mFollowLookAtPlayerOffset, rIter, "FollowLookAtPlayerOffset");
    mIsUseMoveLimitMin.x = tryGetByamlKeyBoolOrFalse(rIter, "IsUseMoveLimitMinX");
    mIsUseMoveLimitMin.y = tryGetByamlKeyBoolOrFalse(rIter, "IsUseMoveLimitMinY");
    mIsUseMoveLimitMin.z = tryGetByamlKeyBoolOrFalse(rIter, "IsUseMoveLimitMinZ");
    mIsUseMoveLimitMax.x = tryGetByamlKeyBoolOrFalse(rIter, "IsUseMoveLimitMaxX");
    mIsUseMoveLimitMax.y = tryGetByamlKeyBoolOrFalse(rIter, "IsUseMoveLimitMaxY");
    mIsUseMoveLimitMax.z = tryGetByamlKeyBoolOrFalse(rIter, "IsUseMoveLimitMaxZ");
    tryGetByamlV3f(&mMoveLimitMin, rIter, "MoveLimitMin");
    tryGetByamlV3f(&mMoveLimitMax, rIter, "MoveLimitMax");
}

/**
 * Takes over the angles of the previous camera unless the start angles are forced.
 * @param pOther Previous camera.
 */
void CameraPoserKinopioBrigade_RS::transferParam(const CameraPoserKinopioBrigade_RS* pOther) {
    if (!mIsTransferAlways && mIsResetForce) {
        return;
    }

    mAngleV = pOther->mAngleV;
    mAngleH = pOther->mAngleH;
    mBaseAngleV = pOther->mBaseAngleV;
    mBaseAngleH = pOther->mBaseAngleH;
    mCurrentBaseAngleV = pOther->mCurrentBaseAngleV;
    mCurrentBaseAngleH = pOther->mCurrentBaseAngleH;
    mDrcAngleV = pOther->mDrcAngleV;
    mDrcAngleH = pOther->mDrcAngleH;
    mSmoothDrcAngleV = pOther->mSmoothDrcAngleV;
    mSmoothDrcAngleH = pOther->mSmoothDrcAngleH;
}

}  // namespace al

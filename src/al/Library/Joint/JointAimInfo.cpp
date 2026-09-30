#include "Library/Joint/JointAimInfo.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/Math/MathUtil.hpp"

namespace al {

/**
 * Constructs aim info facing local Z with a 30 degree circular limit.
 */
JointAimInfo::JointAimInfo() = default;

/**
 * Calculates the aim rotation using the current limit type.
 * @param pQuat Receives the rotation.
 * @param rDir Local direction to aim at.
 */
void JointAimInfo::makeTurnQuat(sead::Quatf* pQuat, const sead::Vector3f& rDir) const {
    switch (mLimitType) {
    case LimitType_Circle:
        makeTurnQuatCircle(pQuat, rDir);
        return;
    case LimitType_Oval:
        makeTurnQuatOval(pQuat, rDir);
        return;
    case LimitType_Rect:
        makeTurnQuatRect(pQuat, rDir);
        return;
    default:
        return;
    }
}

/**
 * Calculates the aim rotation with a circular angle limit.
 * @param pQuat Receives the rotation.
 * @param rDir Local direction to aim at.
 */
void JointAimInfo::makeTurnQuatCircle(sead::Quatf* pQuat, const sead::Vector3f& rDir) const {
    pQuat->set(sead::Quatf::unit);
    turnQuat(pQuat, *pQuat, mBaseAimLocalDir, rDir, sead::Mathf::deg2rad(mLimitDegree[0]));
}

void JointAimInfo::makeTurnQuatOval(sead::Quatf* pQuat, const sead::Vector3f& rDir) const {
    f32 side = rDir.dot(mBaseSideLocalDir);
    f32 sideSq = side * side;
    f32 up = rDir.dot(mBaseUpLocalDir);
    f32 upSq = up * up;
    f32 lengthSq = sideSq + upSq;

    if (isNearZero(lengthSq)) {
        pQuat->set(sead::Quatf::unit);
        return;
    }

    f32 sideLimit = side > 0.0f ? mLimitDegree[0] : mLimitDegree[1];
    f32 upLimit = up > 0.0f ? mLimitDegree[2] : mLimitDegree[3];
    f32 sideLimitSq = sideLimit * sideLimit;
    f32 upLimitSq = upLimit * upLimit;

    f32 limit;

    if (isNearZero(sideLimitSq) && isNearZero(upLimitSq)) {
        limit = 0.0f;
    } else {
        f32 sideRate = sideSq / lengthSq * upLimitSq;
        f32 upRate = upSq / lengthSq * sideLimitSq;
        limit = sead::Mathf::deg2rad(
            sead::Mathf::sqrt((sideLimitSq * upLimitSq) / (upRate + sideRate)));
    }

    pQuat->set(sead::Quatf::unit);
    turnQuat(pQuat, *pQuat, mBaseAimLocalDir, rDir, limit);
}

/**
 * Calculates the aim rotation with separate side and up angle limits.
 * @param pQuat Receives the rotation.
 * @param rDir Local direction to aim at.
 */
void JointAimInfo::makeTurnQuatRect(sead::Quatf* pQuat, const sead::Vector3f& rDir) const {
    f32 sideDegree = calcAngleOnPlaneDegree(mBaseAimLocalDir, rDir, mBaseSideLocalDir);
    f32 upDegree = calcAngleOnPlaneDegree(mBaseAimLocalDir, rDir, mBaseUpLocalDir);
    sideDegree = sideDegree > mLimitDegree[2] ? mLimitDegree[2] : sideDegree;
    sideDegree = sideDegree < -mLimitDegree[3] ? -mLimitDegree[3] : sideDegree;
    upDegree = upDegree > mLimitDegree[0] ? mLimitDegree[0] : upDegree;
    upDegree = upDegree < -mLimitDegree[1] ? -mLimitDegree[1] : upDegree;

    pQuat->set(sead::Quatf::unit);
    rotateQuatRadian(pQuat, *pQuat, mBaseSideLocalDir, sead::Mathf::deg2rad(sideDegree));
    rotateQuatRadian(pQuat, *pQuat, mBaseUpLocalDir, sead::Mathf::deg2rad(upDegree));
}

/**
 * Sets the joint-local aim direction.
 * @param rDir Local aim direction.
 */
void JointAimInfo::setBaseAimLocalDir(const sead::Vector3f& rDir) {
    mBaseAimLocalDir.set(rDir);
}

/**
 * Sets the joint-local up direction.
 * @param rDir Local up direction.
 */
void JointAimInfo::setBaseUpLocalDir(const sead::Vector3f& rDir) {
    mBaseUpLocalDir.set(rDir);
}

/**
 * Sets the joint-local side direction.
 * @param rDir Local side direction.
 */
void JointAimInfo::setBaseSideLocalDir(const sead::Vector3f& rDir) {
    mBaseSideLocalDir.set(rDir);
}

/**
 * Sets the base matrix.
 * @param pMtx Base matrix.
 */
void JointAimInfo::setBaseMtxPtr(const sead::Matrix34f* pMtx) {
    mBaseMtxPtr = pMtx;
}

/**
 * Sets the world position to aim at.
 * @param rPos Target position.
 */
void JointAimInfo::setTargetPos(const sead::Vector3f& rPos) {
    mTargetPos.set(rPos);
}

/**
 * Sets the blend rate, clamped to [0, 1].
 * @param rate New power rate.
 */
void JointAimInfo::setPowerRate(f32 rate) {
    if (rate < 0.0f) {
        rate = 0.0f;
    } else if (rate > 1.0f) {
        rate = 1.0f;
    }

    mPowerRate = rate;
}

/**
 * Uses a circular limit with the same angle in every direction.
 * @param degree Limit angle in degrees.
 */
void JointAimInfo::setLimitDegreeCircle(f32 degree) {
    mLimitDegree[0] = degree;
    mLimitDegree[1] = degree;
    mLimitDegree[2] = degree;
    mLimitDegree[3] = degree;
    mLimitType = LimitType_Circle;
}

/**
 * Uses an elliptical limit.
 * @param sidePlus Limit toward positive side.
 * @param sideMinus Limit toward negative side.
 * @param upPlus Limit toward positive up.
 * @param upMinus Limit toward negative up.
 */
void JointAimInfo::setLimitDegreeOval(f32 sidePlus, f32 sideMinus, f32 upPlus, f32 upMinus) {
    mLimitDegree[0] = sidePlus;
    mLimitDegree[1] = sideMinus;
    mLimitDegree[2] = upPlus;
    mLimitDegree[3] = upMinus;
    mLimitType = LimitType_Oval;
}

/**
 * Uses a rectangular limit.
 * @param sidePlus Limit toward positive side.
 * @param sideMinus Limit toward negative side.
 * @param upPlus Limit toward positive up.
 * @param upMinus Limit toward negative up.
 */
void JointAimInfo::setLimitDegreeRect(f32 sidePlus, f32 sideMinus, f32 upPlus, f32 upMinus) {
    mLimitDegree[0] = sidePlus;
    mLimitDegree[1] = sideMinus;
    mLimitDegree[2] = upPlus;
    mLimitDegree[3] = upMinus;
    mLimitType = LimitType_Rect;
}

/**
 * Sets whether targets behind the joint can be aimed at.
 * @param isEnable Whether back aiming is enabled.
 */
void JointAimInfo::setEnableBackAim(bool isEnable) {
    mIsEnableBackAim = isEnable;
}

/**
 * Increases the blend rate, clamped to [0, 1].
 * @param rate Amount to add.
 */
void JointAimInfo::addPowerRate(f32 rate) {
    setPowerRate(mPowerRate + rate);
}

/**
 * Decreases the blend rate, clamped to [0, 1].
 * @param rate Amount to subtract.
 */
void JointAimInfo::subPowerRate(f32 rate) {
    setPowerRate(mPowerRate - rate);
}

/**
 * Sets the interpolation rate toward the target.
 * @param rate Interpolation rate.
 */
void JointAimInfo::setInterpoleRate(f32 rate) {
    mInterpoleRate = rate;
}

}  // namespace al

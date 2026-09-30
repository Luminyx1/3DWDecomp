#include "Library/Joint/JointLocalDirController.hpp"

#include <math/seadQuat.h>

#include "Library/Joint/JointDirectionInfo.hpp"
#include "Library/Math/MathUtil.hpp"

namespace al {

/**
 * Constructs a controller that turns its joints toward a world direction.
 * @param pInfo Direction parameters.
 */
JointLocalDirController::JointLocalDirController(const JointDirectionInfo* pInfo) : mInfo(pInfo) {}

/**
 * Turns the joint so its base direction faces the target direction, blended by the power rate.
 * @param jointIndex Index of the joint being calculated.
 * @param pMtx Joint matrix to modify.
 */
void JointLocalDirController::calcJointCallback(s32 jointIndex, sead::Matrix34f* pMtx) {
    if (isNearZero(mInfo->mPowerRate)) {
        return;
    }

    sead::Matrix34f invMtx;
    invMtx.setInverse(*pMtx);

    sead::Vector3f localTarget;
    localTarget.setRotated(invMtx, mInfo->mWorldTargetDir);
    sead::Vector3f axis = mInfo->mLocalRotateAxis;
    verticalizeVec(&localTarget, axis, localTarget);
    if (normalizeOrZero(&localTarget)) {
        return;
    }

    sead::Vector3f baseDir = mInfo->mLocalBaseDir;
    sead::Quatf quat = sead::Quatf::unit;
    turnQuat(&quat, quat, baseDir, localTarget, sead::Mathf::deg2rad(mInfo->mLimitDegree));
    quat.slerpTo(sead::Quatf::unit, quat, mInfo->mPowerRate);

    sead::Matrix34f rotateMtx;
    rotateMtx.fromQuat(quat);
    pMtx->setMul(*pMtx, rotateMtx);
}

}  // namespace al

#include "Project/Joint/JointAimController.hpp"

#include "Library/Joint/JointAimInfo.hpp"
#include "Library/Math/MathUtil.hpp"

namespace al {

/**
 * Constructs a controller that turns its joints toward a target position.
 * @param pInfo Aim parameters.
 */
JointAimController::JointAimController(const JointAimInfo* pInfo) : mInfo(pInfo) {
    mQuat.set(sead::Quatf::unit);
}

/**
 * Turns the joint toward the target position, blending with the previous rotation.
 * @param jointIndex Index of the joint being calculated.
 * @param pMtx Joint matrix to modify.
 */
void JointAimController::calcJointCallback(s32 jointIndex, sead::Matrix34f* pMtx) {
    if (isNearZero(mInfo->mPowerRate)) {
        mQuat.set(sead::Quatf::unit);
        return;
    }

    sead::Matrix34f invMtx;

    if (mInfo->mBaseMtxPtr != nullptr) {
        invMtx.setInverse(*mInfo->mBaseMtxPtr);
    } else {
        invMtx.setInverse(*pMtx);
    }

    sead::Vector3f dir;
    dir.setMul(invMtx, mInfo->mTargetPos);

    sead::Quatf quat;
    quat.set(sead::Quatf::unit);

    if (!normalizeOrZero(&dir)) {
        if (mInfo->mIsEnableBackAim) {
            const sead::Vector3f aimDir = mInfo->mBaseAimLocalDir;
            f32 dot = aimDir.dot(dir);

            if (dot < 0.0f) {
                dir -= aimDir * dot * 2.0f;
                normalizeOrZero(&dir);
            }
        }

        mInfo->makeTurnQuat(&quat, dir);
    }

    mQuat.slerpTo(mQuat, quat, mInfo->mInterpoleRate);
    mQuat.slerpTo(sead::Quatf::unit, mQuat, mInfo->mPowerRate);

    sead::Matrix34f rotateMtx;
    rotateMtx.fromQuat(mQuat);
    pMtx->setMul(*pMtx, rotateMtx);
}

}  // namespace al

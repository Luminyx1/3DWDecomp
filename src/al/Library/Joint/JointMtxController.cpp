#include "Library/Joint/JointMtxController.hpp"

namespace al {

/**
 * Constructs a controller that applies an external matrix to its joints.
 * @param pActor Owning actor.
 * @param pMtx Matrix to apply.
 * @param isMulMtx Whether to multiply the joint matrix instead of replacing it.
 */
JointMtxController::JointMtxController(const LiveActor* pActor, const sead::Matrix34f* pMtx,
                                       bool isMulMtx)
    : mActor(pActor), mMtx(pMtx), mIsMulMtx(isMulMtx) {}

/**
 * Multiplies or replaces the joint matrix with the external matrix.
 * @param jointIndex Index of the joint being calculated.
 * @param pMtx Joint matrix to modify.
 */
void JointMtxController::calcJointCallback(s32 jointIndex, sead::Matrix34f* pMtx) {
    if (mIsMulMtx) {
        pMtx->setMul(*pMtx, *mMtx);
    } else {
        *pMtx = *mMtx;
    }
}

}  // namespace al

#include "Library/Joint/JointLocalTransController.hpp"

namespace al {

/**
 * Constructs a controller that applies a local translation to its joints.
 * @param pActor Owning actor.
 * @param pTrans Local translation to apply.
 */
JointLocalTransController::JointLocalTransController(const LiveActor* pActor,
                                                     const sead::Vector3f* pTrans)
    : mActor(pActor), mTrans(pTrans) {}

/**
 * Applies the local translation to the joint matrix.
 * @param jointIndex Index of the joint being calculated.
 * @param pMtx Joint matrix to modify.
 */
void JointLocalTransController::calcJointCallback(s32 jointIndex, sead::Matrix34f* pMtx) {
    sead::Matrix34f transMtx;
    transMtx.makeT(*mTrans);
    pMtx->setMul(*pMtx, transMtx);
}

}  // namespace al

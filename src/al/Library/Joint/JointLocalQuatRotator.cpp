#include "Library/Joint/JointLocalQuatRotator.hpp"

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Model/ModelShapeUtil.hpp"

namespace al {

/**
 * Constructs a rotator that applies a local quaternion rotation to one joint.
 * @param pActor Owning actor.
 * @param pJointName Name of the joint to rotate.
 * @param pQuat Local rotation to apply.
 */
JointLocalQuatRotator::JointLocalQuatRotator(const LiveActor* pActor, const char* pJointName,
                                             const sead::Quatf* pQuat)
    : mActor(pActor), mQuat(pQuat) {
    mJointIndex = getJointIndex(pActor->getModelKeeper(), pJointName);
    appendJointId(mJointIndex);
}

/**
 * Applies the local rotation to the controlled joint.
 * @param jointIndex Index of the joint being calculated.
 * @param pMtx Joint matrix to modify.
 */
void JointLocalQuatRotator::calcJointCallback(s32 jointIndex, sead::Matrix34f* pMtx) {
    if (mJointIndex != jointIndex) {
        return;
    }

    sead::Matrix34f rotateMtx;
    rotateMtx.fromQuat(*mQuat);
    pMtx->setMul(*pMtx, rotateMtx);
}

}  // namespace al

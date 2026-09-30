#include "Library/Joint/JointQuatController.hpp"

#include "Library/Math/MatrixUtil.hpp"

namespace al {

/**
 * Constructs a controller that sets joint rotations from a quaternion.
 * @param pActor Owning actor.
 * @param pQuat Rotation to apply.
 */
JointQuatController::JointQuatController(const LiveActor* pActor, const sead::Quatf* pQuat)
    : mActor(pActor), mQuat(pQuat) {}

/**
 * Replaces the joint rotation with the quaternion, keeping scale and translation.
 * @param jointIndex Index of the joint being calculated.
 * @param pMtx Joint matrix to modify.
 */
void JointQuatController::calcJointCallback(s32 jointIndex, sead::Matrix34f* pMtx) {
    sead::Vector3f trans;
    pMtx->getTranslation(trans);
    sead::Vector3f scale = {1.0f, 1.0f, 1.0f};
    calcMtxScale(&scale, *pMtx);
    pMtx->fromQuat(*mQuat);
    preScaleMtx(pMtx, scale);
    pMtx->setTranslation(trans);
}

}  // namespace al

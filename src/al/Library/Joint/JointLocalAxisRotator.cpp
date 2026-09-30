#include "Library/Joint/JointLocalAxisRotator.hpp"

#include <math/seadQuat.h>

namespace al {

/**
 * Constructs a rotator that turns its joints around an axis.
 * @param pDegree Rotation angle in degrees.
 * @param rAxis Rotation axis.
 * @param isLocal Whether to rotate in joint-local space.
 */
JointLocalAxisRotator::JointLocalAxisRotator(f32* pDegree, const sead::Vector3f& rAxis,
                                             bool isLocal)
    : mAxis(rAxis), mDegree(pDegree), mIsLocal(isLocal) {}

void JointLocalAxisRotator::calcJointCallback(s32 jointIndex, sead::Matrix34f* pMtx) {
    sead::Quatf quat;
    quat.setAxisAngle(mAxis, *mDegree);
    sead::Matrix34f rotateMtx;
    rotateMtx.fromQuat(quat);

    if (mIsLocal) {
        pMtx->setMul(*pMtx, rotateMtx);
    } else {
        sead::Vector3f trans;
        pMtx->getTranslation(trans);
        pMtx->setMul(rotateMtx, *pMtx);
        pMtx->setTranslation(trans);
    }
}

}  // namespace al

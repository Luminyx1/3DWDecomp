#include "Project/Se/SeSource3DRing.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/Math/MathUtil.hpp"
#include "Project/Se/SeSourcePose.hpp"

namespace al {
/**
 * @brief Constructs a 3D sound source emitting from a horizontal ring around the pose.
 * @param pPose The pose providing the ring's center and orientation.
 * @param pRadius Pointer to the ring radius, read every frame.
 * @param pInfo The audio system info.
 */
SeSource3DRing::SeSource3DRing(SeSourcePose3DMtxBase* pPose, const f32* pRadius, AudioSystemInfo* pInfo)
    : SeSource3D("３Ｄリング音源", pPose, pInfo), mPoseMtx(pPose), mRadius(pRadius) {
    mInvPoseMtx.makeIdentity();
}

/**
 * @brief Does nothing: the ring needs no initial position setup.
 */
void SeSource3DRing::calcPositionInitialize() {}

/**
 * @brief Updates the pose and caches the inverse of its matrix.
 */
void SeSource3DRing::calcPositionDynamic() {
    mPoseMtx->update();
    mInvPoseMtx.setInverse(mPoseMtx->get3DMtx());
}

/**
 * @brief Computes the point of the ring closest to the listener.
 * @param rListenerPos The listener position.
 * @return The closest point on the ring, in world space.
 */
const sead::Vector3f* SeSource3DRing::calcPosition(const sead::Vector3f& rListenerPos) {
    sead::Vector3f localPos;
    localPos.setMul(mInvPoseMtx, rListenerPos);
    f32 distance = sead::Mathf::sqrt(localPos.x * localPos.x + localPos.z * localPos.z);
    if (isNearZero(distance, 0.001f)) {
        mCalcPos.set(0.0f, 0.0f, -*mRadius);
    } else {
        f32 scale = *mRadius / distance;
        mCalcPos.set(localPos.x * scale, 0.0f, localPos.z * scale);
    }
    mCalcPos.setMul(mPoseMtx->get3DMtx(), mCalcPos);
    return &mCalcPos;
}
}  // namespace al

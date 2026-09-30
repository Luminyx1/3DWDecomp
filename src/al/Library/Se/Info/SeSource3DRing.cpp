#include "Library/Se/Info/SeSource.hpp"

#include "Library/Math/MathUtil.hpp"
#include "Library/Se/Info/SeSourcePose.hpp"

namespace al {
/**
 * Constructs a ring source.
 * @param pPose Source pose.
 * @param pRadius Ring radius.
 * @param pInfo Audio system information.
 */
SeSource3DRing::SeSource3DRing(SeSourcePose3DMtxBase* pPose, const f32* pRadius, AudioSystemInfo* pInfo)
    : SeSource3D("３Ｄリング音源", pPose, pInfo), mMtxPose(pPose), mRadius(pRadius) {
    mInvMtx.makeIdentity();
}

/**
 * Does nothing.
 */
void SeSource3DRing::calcPositionInitialize() {}

/**
 * Updates the pose and the inverse of its matrix.
 */
void SeSource3DRing::calcPositionDynamic() {
    mMtxPose->update();
    mInvMtx.setInverse(mMtxPose->get3DMtx());
}

/**
 * Calculates the source position nearest to the listener.
 * @param rListenerPos Listener position.
 * @return Source position.
 */
const sead::Vector3f* SeSource3DRing::calcPosition(const sead::Vector3f& rListenerPos) {
    sead::Vector3f localPos;
    localPos.setMul(mInvMtx, rListenerPos);
    sead::Vector2f planePos(localPos.x, localPos.z);
    f32 length = planePos.length();
    if (isNearZero(length, 0.001f)) {
        planePos.set(0.0f, -*mRadius);
    } else {
        planePos *= *mRadius / length;
    }
    mPos.set(planePos.x, 0.0f, planePos.y);
    mPos.setMul(mMtxPose->get3DMtx(), mPos);
    return &mPos;
}
}  // namespace al

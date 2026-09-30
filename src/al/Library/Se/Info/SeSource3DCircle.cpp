#include "Library/Se/Info/SeSource.hpp"

#include "Library/Math/MathUtil.hpp"
#include "Library/Se/Info/SeSourcePose.hpp"

namespace al {
/**
 * Constructs a circle source.
 * @param pPose Source pose.
 * @param pRadius Circle radius.
 * @param pInfo Audio system information.
 * @param isVertical Whether the circle is in the XY plane instead of the XZ plane.
 */
SeSource3DCircle::SeSource3DCircle(SeSourcePose3DMtxBase* pPose, const f32* pRadius, AudioSystemInfo* pInfo,
                                   bool isVertical)
    : SeSource3D("３Ｄ平面円音源", pPose, pInfo), mMtxPose(pPose), mRadius(pRadius), mIsVertical(isVertical) {
    mInvMtx.makeIdentity();
}

/**
 * Does nothing.
 */
void SeSource3DCircle::calcPositionInitialize() {}

/**
 * Updates the pose and the inverse of its matrix.
 */
void SeSource3DCircle::calcPositionDynamic() {
    mMtxPose->update();
    mInvMtx.setInverse(mMtxPose->get3DMtx());
}

const sead::Vector3f* SeSource3DCircle::calcPosition(const sead::Vector3f& rListenerPos) {
    sead::Vector3f localPos;
    localPos.setMul(mInvMtx, rListenerPos);
    if (mIsVertical) {
        sead::Vector2f planePos(localPos.x, localPos.y);
        f32 length = planePos.length();
        if (isNearZero(length, 0.001f)) {
            planePos.set(0.0f, -*mRadius);
        } else if (length >= *mRadius) {
            planePos *= *mRadius / length;
        }
        mPos.set(planePos.x, planePos.y, 0.0f);
    } else {
        sead::Vector2f planePos(localPos.x, localPos.z);
        f32 length = planePos.length();
        if (isNearZero(length, 0.001f)) {
            planePos.set(0.0f, -*mRadius);
        } else if (length >= *mRadius) {
            planePos *= *mRadius / length;
        }
        mPos.set(planePos.x, 0.0f, planePos.y);
    }
    mPos.setMul(mMtxPose->get3DMtx(), mPos);
    return &mPos;
}
}  // namespace al

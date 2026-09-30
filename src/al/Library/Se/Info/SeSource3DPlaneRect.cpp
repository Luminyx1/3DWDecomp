#include "Library/Se/Info/SeSource.hpp"

#include "Library/Se/Info/SeSourcePose.hpp"

namespace al {
/**
 * Constructs a plane rectangle source.
 * @param pPose Source pose.
 * @param pRect Rectangle in the XZ plane of the pose space.
 * @param pInfo Audio system information.
 */
SeSource3DPlaneRect::SeSource3DPlaneRect(SeSourcePose3DMtxBase* pPose, const sead::BoundBox2f* pRect,
                                         AudioSystemInfo* pInfo)
    : SeSource3D("３Ｄ平面長方形音源", pPose, pInfo), mMtxPose(pPose), mRect(pRect) {
    mInvMtx.makeIdentity();
}

/**
 * Does nothing.
 */
void SeSource3DPlaneRect::calcPositionInitialize() {}

/**
 * Updates the pose and the inverse of its matrix.
 */
void SeSource3DPlaneRect::calcPositionDynamic() {
    mMtxPose->update();
    mInvMtx.setInverse(mMtxPose->get3DMtx());
}

/**
 * Calculates the source position nearest to the listener.
 * @param rListenerPos Listener position.
 * @return Source position.
 */
const sead::Vector3f* SeSource3DPlaneRect::calcPosition(const sead::Vector3f& rListenerPos) {
    sead::Vector3f localPos;
    localPos.setMul(mInvMtx, rListenerPos);
    mPos.set(localPos.x, 0.0f, localPos.z);
    const sead::BoundBox2f* rect = mRect;
    if (mPos.x < rect->getMin().x) {
        mPos.x = rect->getMin().x;
    } else if (mPos.x > rect->getMax().x) {
        mPos.x = rect->getMax().x;
    }

    if (mPos.z < rect->getMin().y) {
        mPos.z = rect->getMin().y;
    } else if (mPos.z > rect->getMax().y) {
        mPos.z = rect->getMax().y;
    }

    mPos.setMul(mMtxPose->get3DMtx(), mPos);
    return &mPos;
}
}  // namespace al

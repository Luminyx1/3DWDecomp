#include "Library/Se/Info/SeSource.hpp"

#include "Library/Se/Info/SeSourcePose.hpp"

namespace al {
/**
 * Constructs a point source.
 * @param pPose Source pose.
 * @param pInfo Audio system information.
 */
SeSource3DPoint::SeSource3DPoint(SeSourcePose3D* pPose, AudioSystemInfo* pInfo)
    : SeSource3D("３Ｄ点音源", pPose, pInfo) {}

/**
 * Updates the pose.
 */
void SeSource3DPoint::calcPositionDynamic() {
    mPose->update();
}

/**
 * Gets the pose position.
 * @param rListenerPos Unused.
 * @return Pose position.
 */
const sead::Vector3f* SeSource3DPoint::calcPosition(const sead::Vector3f& rListenerPos) {
    return mPose->get3DPosPtr();
}
}  // namespace al

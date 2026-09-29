#include "Project/Se/SeSource3DPoint.hpp"
#include "Project/Se/SeSourcePose.hpp"

namespace al {
/**
 * @brief Constructs a 3D sound source emitting from a single point.
 * @param pPose The pose providing the point's position.
 * @param pInfo The audio system info.
 */
SeSource3DPoint::SeSource3DPoint(SeSourcePose3D* pPose, AudioSystemInfo* pInfo)
    : SeSource3D("３Ｄ点音源", pPose, pInfo) {}

/**
 * @brief Updates the pose that drives the point's position.
 */
void SeSource3DPoint::calcPositionDynamic() {
    mPose->update();
}

/**
 * @brief Gets the emitting position, which is the pose position regardless of the listener.
 * @param rListenerPos The listener position (unused).
 * @return The point position.
 */
const sead::Vector3f* SeSource3DPoint::calcPosition(const sead::Vector3f& rListenerPos) {
    return mPose->get3DPosPtr();
}
}  // namespace al

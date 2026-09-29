#include "Project/Se/SeSource3DSphere.hpp"
#include "Project/Se/SeSourcePose.hpp"

namespace al {
/**
 * @brief Constructs a 3D sound source filling a sphere around the pose position.
 * @param pPose The pose providing the sphere center.
 * @param pRadius Pointer to the sphere radius, read every frame.
 * @param pInfo The audio system info.
 */
SeSource3DSphere::SeSource3DSphere(SeSourcePose3D* pPose, const f32* pRadius, AudioSystemInfo* pInfo)
    : SeSource3D("３Ｄ球音源", pPose, pInfo), mRadius(pRadius) {}

/**
 * @brief Does nothing: the sphere needs no initial position setup.
 */
void SeSource3DSphere::calcPositionInitialize() {}

/**
 * @brief Updates the pose that drives the sphere center.
 */
void SeSource3DSphere::calcPositionDynamic() {
    mPose->update();
}

/**
 * @brief Computes the point of the sphere closest to the listener.
 * @param rListenerPos The listener position.
 * @return The listener position if it is inside the sphere, otherwise the nearest point on its surface.
 */
const sead::Vector3f* SeSource3DSphere::calcPosition(const sead::Vector3f& rListenerPos) {
    sead::Vector3f dir = rListenerPos - mPose->get3DPos();
    f32 distance = dir.length();
    if (distance <= *mRadius) {
        mCalcPos = rListenerPos;
    } else {
        dir *= *mRadius / distance;
        mCalcPos = mPose->get3DPos() + dir;
    }
    return &mCalcPos;
}
}  // namespace al

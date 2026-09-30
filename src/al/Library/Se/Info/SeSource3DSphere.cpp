#include "Library/Se/Info/SeSource.hpp"

#include "Library/Se/Info/SeSourcePose.hpp"

namespace al {
/**
 * Constructs a sphere source.
 * @param pPose Source pose.
 * @param pRadius Sphere radius.
 * @param pInfo Audio system information.
 */
SeSource3DSphere::SeSource3DSphere(SeSourcePose3D* pPose, const f32* pRadius, AudioSystemInfo* pInfo)
    : SeSource3D("３Ｄ球音源", pPose, pInfo), mRadius(pRadius) {}

/**
 * Does nothing.
 */
void SeSource3DSphere::calcPositionInitialize() {}

/**
 * Updates the pose.
 */
void SeSource3DSphere::calcPositionDynamic() {
    mPose->update();
}

/**
 * Calculates the source position nearest to the listener.
 * @param rListenerPos Listener position.
 * @return Source position.
 */
const sead::Vector3f* SeSource3DSphere::calcPosition(const sead::Vector3f& rListenerPos) {
    sead::Vector3f dir = rListenerPos - mPose->get3DPos();
    f32 length = dir.length();
    if (length <= *mRadius) {
        mPos = rListenerPos;
    } else {
        dir *= *mRadius / length;
        mPos = dir + mPose->get3DPos();
    }
    return &mPos;
}
}  // namespace al

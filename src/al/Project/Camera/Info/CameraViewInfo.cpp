#include "Project/Camera/Info/CameraViewInfo.hpp"

#include "Library/Projection/Projection.hpp"

namespace al {
/**
 * @brief Creates the view info from the objects describing the view.
 * @param index The index of the view.
 * @param rLookAtCam The camera of the view.
 * @param rProjection The projection of the view.
 * @param rFlag The flags of the view.
 * @param rOrthoProjectionInfo The orthographic projection settings of the view.
 */
CameraViewInfo::CameraViewInfo(s32 index, const sead::LookAtCamera& rLookAtCam, Projection& rProjection,
                               const CameraViewFlag& rFlag, const OrthoProjectionInfo& rOrthoProjectionInfo)
    : mIndex(index), mLookAtCam(rLookAtCam), mProjection(rProjection), mFlag(rFlag),
      mOrthoProjectionInfo(rOrthoProjectionInfo) {}

/**
 * @brief Gets the sead projection of the view.
 * @return The sead projection.
 */
sead::Projection& CameraViewInfo::getProjectionSead() {
    return mProjection.getProjectionSead();
}

/**
 * @brief Gets the sead projection of the view.
 * @return The sead projection.
 */
const sead::Projection& CameraViewInfo::getProjectionSead() const {
    return mProjection.getProjectionSead();
}

/**
 * @brief Gets the projection matrix of the view.
 * @return The projection matrix.
 */
const sead::Matrix44f* CameraViewInfo::getProjMtx() const {
    return &mProjection.getProjMtx();
}

/**
 * @brief Gets the aspect ratio of the view.
 * @return The aspect ratio.
 */
f32 CameraViewInfo::getAspect() const {
    return mProjection.getAspect();
}

/**
 * @brief Gets the near clip distance of the view.
 * @return The near clip distance.
 */
f32 CameraViewInfo::getNear() const {
    return mProjection.getNear();
}

/**
 * @brief Gets the far clip distance of the view.
 * @return The far clip distance.
 */
f32 CameraViewInfo::getFar() const {
    return mProjection.getFar();
}
}  // namespace al

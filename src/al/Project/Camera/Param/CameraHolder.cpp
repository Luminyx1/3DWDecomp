#include "Project/Camera/Param/CameraHolder.hpp"

#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Project/Camera/Param/CameraInfo.hpp"

namespace al {
/**
 * Creates an empty holder with room for 128 cameras.
 */
CameraHolder::CameraHolder() {
    mCameraInfos = new CameraInfo*[128];
}

/**
 * Registers a placed camera.
 * @param pPlacementId The placement id of the camera.
 * @param pPoser The poser of the camera.
 * @param priority The priority of the camera.
 */
void CameraHolder::setCameraInfos(const PlacementId* pPlacementId, CameraPoser* pPoser, s32 priority) {
    mCameraInfos[mNumCameras] = new CameraInfo(pPlacementId, pPoser, priority);
    mNumCameras++;
}

/**
 * Gets a camera by its index.
 * @param index The index of the camera.
 * @return The poser of the camera.
 */
CameraPoser* CameraHolder::getCameraByIndex(s32 index) const {
    return mCameraInfos[index]->mPoser;
}

/**
 * Looks for a camera by its placement id.
 * @param pPlacementId The placement id of the camera.
 * @return The poser of the camera, or null if there is none.
 */
CameraPoser* CameraHolder::getCameraById(const PlacementId* pPlacementId) const {
    for (s32 i = 0; i < mNumCameras; i++) {
        if (isEqualPlacementID(*mCameraInfos[i]->mPlacementId, *pPlacementId)) {
            return mCameraInfos[i]->mPoser;
        }
    }
    return nullptr;
}

/**
 * Looks for the info of a camera by its placement id.
 * @param pPlacementId The placement id of the camera.
 * @return The info of the camera, or null if there is none.
 */
CameraInfo* CameraHolder::getCameraInfoById(const PlacementId* pPlacementId) const {
    for (s32 i = 0; i < mNumCameras; i++) {
        if (isEqualPlacementID(*mCameraInfos[i]->mPlacementId, *pPlacementId)) {
            return mCameraInfos[i];
        }
    }
    return nullptr;
}

/**
 * Checks whether a camera with the placement id exists.
 * @param pPlacementId The placement id of the camera.
 * @return True if there is such a camera.
 */
bool CameraHolder::isExistCameraId(const PlacementId* pPlacementId) const {
    for (s32 i = 0; i < mNumCameras; i++) {
        if (isEqualPlacementID(*mCameraInfos[i]->mPlacementId, *pPlacementId)) {
            return true;
        }
    }
    return false;
}
}  // namespace al

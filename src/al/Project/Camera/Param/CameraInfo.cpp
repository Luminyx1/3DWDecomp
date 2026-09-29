#include "Project/Camera/Param/CameraInfo.hpp"

#include "Library/Play/Placement/PlacementId.hpp"

namespace al {
/**
 * @brief Creates the info, keeping its own copy of the placement id.
 * @param pPlacementId The placement id of the camera.
 * @param pPoser The poser that controls the camera.
 * @param priority The priority of the camera.
 */
CameraInfo::CameraInfo(const PlacementId* pPlacementId, CameraPoser* pPoser, s32 priority)
    : mPlacementId(nullptr), mPoser(pPoser), mPriority(priority) {
    mPlacementId = new PlacementId(*pPlacementId);
}
}  // namespace al

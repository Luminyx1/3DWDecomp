#include "Library/Play/Camera/CameraPoserProgramable.hpp"

#include <gfx/seadCamera.h>

#include "Library/Play/Placement/PlacementId.hpp"

namespace al {
/**
 * Creates the camera, keeping its own copy of the placement id.
 * @param pPlacementId The placement id of the camera.
 */
CameraPoserProgramable::CameraPoserProgramable(const PlacementId* pPlacementId) {
    mName = "Programable";
    mPlacementId = new PlacementId(*pPlacementId);
}

/**
 * Writes the pose set from code to the camera.
 * @param pCamera The camera to write to.
 */
void CameraPoserProgramable::makeLookAtCamera(sead::LookAtCamera* pCamera) const {
    pCamera->setPos(mProgramCameraPos);
    pCamera->setAt(mLookAtPos);
    pCamera->setUp(mProgramCameraUp);
    pCamera->normalizeUp();
}
}  // namespace al

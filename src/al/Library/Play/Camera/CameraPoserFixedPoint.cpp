#include "Library/Play/Camera/CameraPoserFixedPoint.hpp"

#include <gfx/seadCamera.h>

#include "Library/Math/MathUtil.hpp"
#include "Library/Play/Placement/PlacementId.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Camera/Param/CameraFunction.hpp"
#include "Library/Camera/PlayerWatcher.hpp"

namespace al {
/**
 * Creates the camera, keeping its own copy of the placement id.
 * @param pPlayerWatcher The watcher of the players to look at.
 * @param pPlacementId The placement id of the camera.
 */
CameraPoserFixedPoint::CameraPoserFixedPoint(const PlayerWatcher* pPlayerWatcher, const PlacementId* pPlacementId)
    : mPlayerWatcher(pPlayerWatcher) {
    mName = "FixedPoint";
    mPlacementId = new PlacementId(*pPlacementId);
}

/**
 * Places the camera at its position in the zone and turns it towards the top player.
 */
void CameraPoserFixedPoint::update() {
    sead::Vector3f playerPos = mPlayerWatcher->getTopPlayerPos();
    CameraFunction::calcPosFromZoneToRoot(&mCameraPos, mLocalCameraPos, mZoneMtx);

    sead::Vector3f dir = playerPos - mCameraPos;
    normalize(&dir);
    mLookAtPos = dir * 1800.0f + mCameraPos;
}

/**
 * Writes the pose of the camera.
 * @param pCamera The camera to write to.
 */
void CameraPoserFixedPoint::makeLookAtCamera(sead::LookAtCamera* pCamera) const {
    pCamera->setPos(mCameraPos);
    pCamera->setAt(mLookAtPos);
    pCamera->setUp(mCameraUp);
    pCamera->normalizeUp();
}

/**
 * Reads the camera position and the interpolation setting.
 * @param pIter The camera parameters.
 */
void CameraPoserFixedPoint::loadParam(const ByamlIter* pIter) {
    CameraPoser::loadParam(pIter);

    ByamlIter posIter;

    if (pIter->tryGetIterByKey(&posIter, "CameraPos")) {
        posIter.tryGetFloatByKey(&mLocalCameraPos.x, "X");
        posIter.tryGetFloatByKey(&mLocalCameraPos.y, "Y");
        posIter.tryGetFloatByKey(&mLocalCameraPos.z, "Z");
    }

    pIter->tryGetBoolByKey(&mIsNoNormalInterpole, "IsNoNormalInterpole");
    CameraPoser::loadParam(pIter);
}
}  // namespace al

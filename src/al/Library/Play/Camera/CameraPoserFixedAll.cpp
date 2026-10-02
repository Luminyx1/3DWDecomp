#include "Library/Play/Camera/CameraPoserFixedAll.hpp"

#include <gfx/seadCamera.h>
#include <math/seadMatrix.h>
#include <math/seadQuat.h>

#include "Library/Play/Placement/PlacementId.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Camera/Param/CameraFunction.hpp"

namespace al {
/**
 * Creates the camera, keeping its own copy of the placement id.
 * @param rParam The default distance and angles.
 * @param pPlayerWatcher The watcher of the players.
 * @param pPlacementId The placement id of the camera.
 */
CameraPoserFixedAll::CameraPoserFixedAll(const CameraPoserFixedAllParam& rParam,
                                         const PlayerWatcher* pPlayerWatcher,
                                         const PlacementId* pPlacementId)
    : mParam(rParam), mPlayerWatcher(pPlayerWatcher) {
    mName = "FixedAll";
    mPlacementId = new PlacementId(*pPlacementId);
}

/**
 * Places the look at position in the zone.
 */
void CameraPoserFixedAll::update() {
    CameraFunction::calcPosFromZoneToRoot(&mLookAtPos, mLocalLookAtPos, mZoneMtx);
}

/**
 * Writes the pose of the camera, rotated around the look at position by the fixed angles.
 * @param pCamera The camera to write to.
 */
void CameraPoserFixedAll::makeLookAtCamera(sead::LookAtCamera* pCamera) const {
    f32 angleV = mParam.mAngleV;
    sead::Vector3f front = sead::Vector3f::ez;
    f32 angleH = CameraFunction::calcAngleHFromZoneToRoot(mParam.mAngleH, mZoneMtx);

    sead::Quatf quatV;
    quatV.setAxisAngle(-sead::Vector3f::ex, angleV);
    sead::Matrix34f mtxV;
    mtxV.fromQuat(quatV);

    sead::Quatf quatH;
    quatH.setAxisAngle(sead::Vector3f::ey, angleH);
    sead::Matrix34f mtxH;
    mtxH.fromQuat(quatH);

    sead::Matrix34f mtx;
    mtx.setMul(mtxH, mtxV);
    front.mul(mtx);

    pCamera->setPos(mLookAtPos + mParam.mDistance * front);
    pCamera->setAt(mLookAtPos);
    pCamera->setUp(mCameraUp);
    pCamera->normalizeUp();
}

/**
 * Reads the distance, the angles and the look at position.
 * @param pIter The camera parameters.
 */
void CameraPoserFixedAll::loadParam(const ByamlIter* pIter) {
    CameraPoser::loadParam(pIter);
    pIter->tryGetFloatByKey(&mParam.mDistance, "Distance");
    pIter->tryGetFloatByKey(&mParam.mAngleV, "AngleV");
    pIter->tryGetFloatByKey(&mParam.mAngleH, "AngleH");
    pIter->tryGetFloatByKey(&mLocalLookAtPos.x, "LookAtPosX");
    pIter->tryGetFloatByKey(&mLocalLookAtPos.y, "LookAtPosY");
    pIter->tryGetFloatByKey(&mLocalLookAtPos.z, "LookAtPosZ");
}

/**
 * Creates the parameters with the default distance and vertical angle.
 */
CameraPoserFixedAllParam::CameraPoserFixedAllParam()
    : mDistance(1800.0f), mAngleV(30.0f), mAngleH(0.0f) {}
}  // namespace al

#include "Library/Play/Camera/CameraPoserParallelOnRail.hpp"

#include <gfx/seadCamera.h>
#include <gfx/seadProjection.h>
#include <gfx/seadViewport.h>
#include <math/seadMathCalcCommon.h>
#include <math/seadMatrix.h>
#include <math/seadQuat.h>

#include "Library/Math/MatrixUtil.hpp"
#include "Library/Obj/PlayerWatcher.hpp"
#include "Library/Play/Placement/PlacementId.hpp"
#include "Library/Rail/RailKeeper.hpp"
#include "Library/Rail/RailRider.hpp"
#include "Library/Rail/RailUtil.hpp"
#include "Library/Screen/ScreenFunction.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Camera/ControlAngleParam.hpp"

namespace {
/**
 * Extends a box so that it contains a position.
 * @param pBox The box to extend.
 * @param rPos The position to include.
 */
inline void includePos(sead::BoundBox2f* pBox, const sead::Vector2f& rPos) {
    sead::Vector2f min = pBox->getMin();
    sead::Vector2f max = pBox->getMax();

    if (rPos.x < min.x) {
        min.x = rPos.x;
    }

    if (rPos.y < min.y) {
        min.y = rPos.y;
    }

    if (rPos.x > max.x) {
        max.x = rPos.x;
    }

    if (rPos.y > max.y) {
        max.y = rPos.y;
    }

    pBox->set(min, max);
}
}  // namespace

namespace al {
/**
 * Creates the camera, keeping its own copy of the placement id.
 * @param rParam The default camera parameters.
 * @param pPlayerWatcher The watcher of the players.
 * @param pProjection The projection used to place the players on the screen.
 * @param pPlacementId The placement id of the camera.
 */
CameraPoserParallelOnRail::CameraPoserParallelOnRail(const CameraPoserParallelOnRailParam& rParam,
                                                     const PlayerWatcher* pPlayerWatcher,
                                                     const sead::PerspectiveProjection* pProjection,
                                                     const PlacementId* pPlacementId)
    : mParam(rParam), mPlayerWatcher(pPlayerWatcher), mProjection(pProjection) {
    mName = "ParallelOnRail";
    mPlacementId = new PlacementId(*pPlacementId);
    mLookAtCamera = new sead::LookAtCamera;
    _88 = true;
    mControlAngleParam = new ControlAngleParam();
}

/**
 * Creates the rail the look at position moves along.
 * @param pInfo The placement info of the camera.
 */
void CameraPoserParallelOnRail::initRail(const PlacementInfo* pInfo) {
    mRailKeeper = tryCreateRailKeeper(*pInfo, "Rail");
}

/**
 * Moves the look at position along the rail following the top player, then adjusts the camera
 * distance so that all players fit in the screen.
 */
void CameraPoserParallelOnRail::update() {
    if (mRailKeeper == nullptr) {
        return;
    }

    switch (mRailType) {
    case RailType_CameraPos: {
        sead::Vector3f dir = sead::Vector3f::ez;

        switch (mAngleType) {
        case AngleType_Fixed:
            calcDirAngleTypeFixed(&dir);
            break;
        case AngleType_Rail:
            calcDirAngleTypeRail(&dir);
            break;
        }

        sead::Vector3f cameraPos = sead::Vector3f::zero;
        const sead::Vector3f& topPlayerPos = mPlayerWatcher->getTopPlayerPos();
        cameraPos = dir * mParam.mDistance + topPlayerPos;
        calcNearestRailPos(&mLookAtPos, mRailKeeper, cameraPos);
        mRailKeeper->getRailRider()->moveToNearestRail(mLookAtPos);
        break;
    }
    case RailType_LookAt: {
        sead::Vector3f lookAtPos = mPlayerWatcher->getTopPlayerPos();

        if (mPlayerWatcher->isExistAdditionalCameraLookAtPos()) {
            const sead::Vector3f& additionalPos = mPlayerWatcher->getAdditionalCameraLookAtPos();
            lookAtPos.x = additionalPos.x;
            lookAtPos.y = additionalPos.y;
            lookAtPos.z = additionalPos.z;
        }

        calcNearestRailPos(&mLookAtPos, mRailKeeper, lookAtPos);
        mRailKeeper->getRailRider()->moveToNearestRail(mLookAtPos);
        break;
    }
    }

    sead::Matrix34f mtx = mLookAtCamera->getMatrix();
    sead::Vector3f front;
    mtx.getBase(front, 2);
    makeMtxUpFront(&mtx, sead::Vector3f::ey, front);
    sead::Matrix34f invMtx;
    invMtx.setInverse(mtx);

    sead::Vector3f localPos;
    localPos.setMul(mtx, mLookAtPos);
    localPos += mParam.mLookAtOffset;
    mLookAtPos.setMul(invMtx, localPos);
    calcCameraDistance();
}

/**
 * Rotates a direction by the fixed vertical and horizontal camera angles.
 * @param pDir The direction to rotate.
 */
void CameraPoserParallelOnRail::calcDirAngleTypeFixed(sead::Vector3f* pDir) const {
    sead::Quatf quatV;
    quatV.setAxisAngle(-sead::Vector3f::ex, mParam.mAngleV);
    sead::Matrix34f mtxV;
    mtxV.fromQuat(quatV);

    sead::Quatf quatH;
    quatH.setAxisAngle(sead::Vector3f::ey, mParam.mAngleH);
    sead::Matrix34f mtxH;
    mtxH.fromQuat(quatH);

    sead::Matrix34f mtx;
    mtx.setMul(mtxH, mtxV);
    pDir->setMul(mtx, *pDir);
}

/**
 * Rotates a direction by the vertical camera angle and the horizontal angle of the rail.
 * @param pDir The direction to rotate.
 */
void CameraPoserParallelOnRail::calcDirAngleTypeRail(sead::Vector3f* pDir) const {
    const sead::Vector3f& railDir = mRailKeeper->getRailRider()->getDirection();
    f32 angleH = sead::Mathf::rad2deg(sead::Mathf::atan2(-railDir.z, railDir.x)) - 90.0f;

    sead::Quatf quatH;
    quatH.setAxisAngle(sead::Vector3f::ey, angleH);
    sead::Matrix34f mtxH;
    mtxH.fromQuat(quatH);

    sead::Quatf quatV;
    quatV.setAxisAngle(-sead::Vector3f::ex, mParam.mAngleV);
    sead::Matrix34f mtxV;
    mtxV.fromQuat(quatV);

    pDir->setMul(mtxH * mtxV, *pDir);
}

/**
 * Moves the camera away from the minimum distance until all players fit in the layout box.
 */
void CameraPoserParallelOnRail::calcCameraDistance() {
    f32 prevDistance = mParam.mDistance;
    mParam.mDistance = mParam.mDistanceMin;
    updateLookAtCamera();

    while (mParam.mDistance < mParam.mDistanceMax && !isAllPlayerInLayoutBox()) {
        mParam.mDistance += 600.0f;
        updateLookAtCamera();

        if (mParam.mDistance >= mParam.mDistanceMax) {
            mParam.mDistance = mParam.mDistanceMax;
            break;
        }
    }

    if (prevDistance < mParam.mDistance) {
        mDistanceKeepFrame = 0;
    } else if (mDistanceKeepFrame < 240) {
        mParam.mDistance = prevDistance;
        mDistanceKeepFrame++;
    }
}

/**
 * Updates the internal camera used to place the players on the screen.
 */
void CameraPoserParallelOnRail::updateLookAtCamera() {
    makeLookAtCamera(mLookAtCamera);
    mLookAtCamera->updateViewMatrix();
}

/**
 * @return Whether all players are inside the layout box of the screen.
 */
bool CameraPoserParallelOnRail::isAllPlayerInLayoutBox() {
    sead::BoundBox2f box;
    calcLayoutBoxIncludeAllPlayer(&box);

    if (box.getSizeX() <= mParam.mLayoutPosMaxX * 2) {
        return box.getMax().y < mParam.mLayoutPosMaxTopY &&
               box.getMin().y > -mParam.mLayoutPosMaxBottomY;
    }

    return false;
}

/**
 * Writes the pose of the camera depending on the rail type.
 * @param pCamera The camera to write to.
 */
void CameraPoserParallelOnRail::makeLookAtCamera(sead::LookAtCamera* pCamera) const {
    switch (mRailType) {
    case RailType_CameraPos:
        makeLookAtCameraRailTypeCameraPos(pCamera);
        break;
    case RailType_LookAt:
        makeLookAtCameraRailTypeLookAt(pCamera);
        break;
    }
}

/**
 * Writes the pose of the camera with the camera placed on the rail.
 * @param pCamera The camera to write to.
 */
void CameraPoserParallelOnRail::makeLookAtCameraRailTypeCameraPos(
    sead::LookAtCamera* pCamera) const {
    sead::Vector3f cameraPos = mLookAtPos;
    sead::Vector3f dir = sead::Vector3f::ez;

    switch (mAngleType) {
    case AngleType_Fixed:
        calcDirAngleTypeFixed(&dir);
        break;
    case AngleType_Rail:
        calcDirAngleTypeRail(&dir);
        break;
    }

    sead::Vector3f at = cameraPos - dir * mParam.mDistance;
    pCamera->setPos(cameraPos);
    pCamera->setAt(at);
    pCamera->setUp(mCameraUp);
    pCamera->normalizeUp();
}

/**
 * Writes the pose of the camera with the look at position placed on the rail.
 * @param pCamera The camera to write to.
 */
void CameraPoserParallelOnRail::makeLookAtCameraRailTypeLookAt(sead::LookAtCamera* pCamera) const {
    sead::Vector3f dir = sead::Vector3f::ez;
    sead::Vector3f lookAtPos = mLookAtPos;

    switch (mAngleType) {
    case AngleType_Fixed:
        calcDirAngleTypeFixed(&dir);
        break;
    case AngleType_Rail:
        calcDirAngleTypeRail(&dir);
        break;
    }

    sead::Vector3f cameraPos = dir * mParam.mDistance + lookAtPos;
    pCamera->setAt(lookAtPos);
    pCamera->setPos(cameraPos);
    pCamera->setUp(mCameraUp);
    pCamera->normalizeUp();
}

/**
 * Calculates the screen box that contains all alive players and the additional look at
 * position.
 * @param pBox Receives the box in screen coordinates.
 */
void CameraPoserParallelOnRail::calcLayoutBoxIncludeAllPlayer(sead::BoundBox2f* pBox) {
    sead::BoundBox2f box;

    for (s32 i = 0; i < mPlayerWatcher->getPlayerNum(); i++) {
        if (!mPlayerWatcher->isPlayerAlive(i)) {
            continue;
        }

        sead::Viewport viewport(0.0f, 0.0f, static_cast<u32>(getDisplayWidth()),
                                static_cast<u32>(getDisplayHeight()));
        sead::Vector3f playerPos = mPlayerWatcher->getPlayerPos(i);
        sead::Vector3f playerLookAtPos;
        mPlayerWatcher->getPlayerLookAtPos(&playerLookAtPos, i);
        playerPos.y = playerLookAtPos.y;

        sead::Vector2f screenPos;
        mLookAtCamera->projectByMatrix(&screenPos, playerPos, *mProjection, viewport);
        includePos(&box, screenPos);
    }

    if (mPlayerWatcher->isExistAdditionalCameraLookAtPos()) {
        sead::Viewport viewport(0.0f, 0.0f, static_cast<u32>(getDisplayWidth()),
                                static_cast<u32>(getDisplayHeight()));
        sead::Vector2f screenPos;
        mLookAtCamera->projectByMatrix(&screenPos, mPlayerWatcher->getAdditionalCameraLookAtPos(),
                                       *mProjection, viewport);
        includePos(&box, screenPos);
    }

    *pBox = box;
}

/**
 * Reads the rail type, angle type, distance, angles and layout box of the camera.
 * @param pIter The camera parameters.
 */
void CameraPoserParallelOnRail::loadParam(const ByamlIter* pIter) {
    CameraPoser::loadParam(pIter);
    pIter->tryGetIntByKey(&mRailType, "CameraRailType");
    pIter->tryGetIntByKey(&mAngleType, "CameraAngleType");
    pIter->tryGetFloatByKey(&mParam.mDistanceMin, "DistanceMin");
    pIter->tryGetFloatByKey(&mParam.mDistanceMax, "DistanceMax");
    pIter->tryGetFloatByKey(&mParam.mAngleV, "AngleV");
    pIter->tryGetFloatByKey(&mParam.mAngleH, "AngleH");
    pIter->tryGetFloatByKey(&mParam.mLayoutPosMaxX, "LayoutPosMaxX");
    pIter->tryGetFloatByKey(&mParam.mLayoutPosMaxTopY, "LayoutPosMaxTopY");
    pIter->tryGetFloatByKey(&mParam.mLayoutPosMaxBottomY, "LayoutPosMaxBottomY");
    pIter->tryGetIntByKey(&mInterpolationFrame, "InterpolationFrame");

    ByamlIter offsetIter;

    if (pIter->tryGetIterByKey(&offsetIter, "LookAtOffset")) {
        offsetIter.tryGetFloatByKey(&mParam.mLookAtOffset.x, "X");
        offsetIter.tryGetFloatByKey(&mParam.mLookAtOffset.y, "Y");
        offsetIter.tryGetFloatByKey(&mParam.mLookAtOffset.z, "Z");
    }

    pIter->tryGetBoolByKey(&mIsNoNormalInterpole, "IsNoNormalInterpole");
}

/**
 * Creates the default parameters.
 */
CameraPoserParallelOnRailParam::CameraPoserParallelOnRailParam() = default;
}  // namespace al

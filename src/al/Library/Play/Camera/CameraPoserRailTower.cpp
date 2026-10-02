#include "Library/Play/Camera/CameraPoserRailTower.hpp"

#include <gfx/seadCamera.h>
#include <math/seadMathCalcCommon.h>

#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/Nerve.hpp"
#include "Library/Nerve/NerveKeeper.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementId.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Library/Rail/Rail.hpp"
#include "Library/Rail/RailKeeper.hpp"
#include "Library/Rail/RailRider.hpp"
#include "Library/Rail/RailUtil.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Camera/Param/CameraFunction.hpp"

namespace {
using namespace al;

class CameraPoserRailTowerNrvWait : public al::Nerve {
public:
    void execute(al::NerveKeeper* pKeeper) const override {
        reinterpret_cast<CameraPoserRailTower*>(pKeeper->mKeeperUser)->exeWait();
    }
};

class CameraPoserRailTowerNrvMove : public al::Nerve {
public:
    void execute(al::NerveKeeper* pKeeper) const override {
        reinterpret_cast<CameraPoserRailTower*>(pKeeper->mKeeperUser)->exeMove();
    }
};

const CameraPoserRailTowerNrvWait NrvCameraPoserRailTowerWait{};
const CameraPoserRailTowerNrvMove NrvCameraPoserRailTowerMove{};
}  // namespace

namespace al {
/**
 * Creates the camera, keeping its own copy of the placement id.
 * @param rParam The default camera parameters.
 * @param pPlacementId The placement id of the camera.
 */
CameraPoserRailTower::CameraPoserRailTower(const CameraPoserRailTowerParam& rParam,
                                           const PlacementId* pPlacementId)
    : mParam(rParam) {
    mName = "RailTower";
    mPlacementId = new PlacementId(*pPlacementId);
}

/**
 * Creates the rail the camera moves along and reads the move parameters of its first point.
 * @param pInfo The placement info of the camera.
 */
void CameraPoserRailTower::initRail(const PlacementInfo* pInfo) {
    mRailKeeper = tryCreateRailKeeper(*pInfo, "RailWithMoveParameter");
    mNerveKeeper = new NerveKeeper(this, &NrvCameraPoserRailTowerWait, 0);

    const PlacementInfo& pointInfo = *mRailKeeper->getRail()->getRailPoint(0);
    tryGetArg(&mSpeed, pointInfo, "Speed");
    tryGetArg(&mWaitTime, pointInfo, "WaitTime");
    tryGetArg(&mOffsetY, pointInfo, "OffsetY");
    tryGetArg(&mAngleV, pointInfo, "AngleV");
    tryGetArg(&mDistance, pointInfo, "Distance");
}

/**
 * Moves the rail rider back to the start of the rail.
 */
void CameraPoserRailTower::start() {
    mRailKeeper->getRailRider()->setCoord(0.0f);
}

/**
 * Updates the tower axis in root space, advances the rail movement and looks at the rail
 * position raised by the height offset.
 */
void CameraPoserRailTower::update() {
    CameraFunction::calcPosFromZoneToRoot(&mAxisPosRoot, mParam.axisPos, mZoneMtx);
    mNerveKeeper->update();
    mLookAtPos = mRailKeeper->getRailRider()->getPosition();
    mLookAtPos.y += mOffsetY;
}

/**
 * Starts moving once the wait time at a rail point has passed.
 */
void CameraPoserRailTower::exeWait() {
    if (mNerveKeeper->getCurrentStep() == mWaitTime) {
        mNerveKeeper->setNerve(&NrvCameraPoserRailTowerMove);
        mRailKeeper->getRailRider()->setSpeed(mSpeed);
    }
}

/**
 * Moves along the rail and applies the move parameters of each newly reached rail point.
 */
void CameraPoserRailTower::exeMove() {
    if (mIsSnapshotMode) {
        return;
    }

    RailRider* rider = mRailKeeper->getRailRider();
    Rail* rail = mRailKeeper->getRail();
    rider->move();
    s32 sectionIndex = rail->getIncludedSectionIndex(rider->getCoord());

    if (mSectionIndex < sectionIndex) {
        const PlacementInfo& pointInfo = *rail->getRailPoint(sectionIndex);
        tryGetArg(&mSpeed, pointInfo, "Speed");
        tryGetArg(&mWaitTime, pointInfo, "WaitTime");
        tryGetArg(&mOffsetY, pointInfo, "OffsetY");
        tryGetArg(&mAngleV, pointInfo, "AngleV");
        tryGetArg(&mDistance, pointInfo, "Distance");
        rider->setSpeed(mSpeed);
        mNerveKeeper->setNerve(&NrvCameraPoserRailTowerWait);
        mSectionIndex = sectionIndex;
    }
}

/**
 * Writes the pose of the camera, looking at the rail position from outside the tower axis.
 * @param pCamera The camera to write to.
 */
void CameraPoserRailTower::makeLookAtCamera(sead::LookAtCamera* pCamera) const {
    sead::Vector3f dir = mLookAtPos - mAxisPosRoot;
    verticalizeVec(&dir, sead::Vector3f::ey, dir);
    normalizeOrZero(&dir);

    f32 dirX = dir.x;
    f32 dirZ = dir.z;
    f32 horizontal = sead::Mathf::cos(sead::Mathf::deg2rad(mAngleV)) * mDistance;
    f32 offsetX = dirX * horizontal;
    f32 offsetZ = dirZ * horizontal;
    f32 offsetY = sead::Mathf::sin(sead::Mathf::deg2rad(mAngleV)) * mDistance;
    sead::Vector3f offset = {offsetX, offsetY, offsetZ};

    pCamera->setPos(offset + mLookAtPos);
    pCamera->setAt(mLookAtPos);
    pCamera->setUp(mCameraUp);
    pCamera->normalizeUp();
}

/**
 * Reads the position of the tower axis.
 * @param pIter The camera parameters.
 */
void CameraPoserRailTower::loadParam(const ByamlIter* pIter) {
    CameraPoser::loadParam(pIter);

    ByamlIter axisIter;

    if (pIter->tryGetIterByKey(&axisIter, "AxisPos")) {
        axisIter.tryGetFloatByKey(&mParam.axisPos.x, "X");
        axisIter.tryGetFloatByKey(&mParam.axisPos.y, "Y");
        axisIter.tryGetFloatByKey(&mParam.axisPos.z, "Z");
    }
}

/**
 * Creates the default rail tower camera parameters.
 */
CameraPoserRailTowerParam::CameraPoserRailTowerParam() = default;
}  // namespace al

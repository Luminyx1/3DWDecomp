#include "Library/Play/Camera/CameraPoserRail.hpp"

#include <gfx/seadCamera.h>
#include <math/seadMatrix.h>
#include <math/seadQuat.h>

#include "Library/Nerve/Nerve.hpp"
#include "Library/Nerve/NerveKeeper.hpp"
#include "Library/Obj/PlayerWatcher.hpp"
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

class CameraPoserRailNrvWait : public al::Nerve {
public:
    void execute(al::NerveKeeper* pKeeper) const override {
        reinterpret_cast<CameraPoserRail*>(pKeeper->mKeeperUser)->exeWait();
    }
};

class CameraPoserRailNrvMove : public al::Nerve {
public:
    void execute(al::NerveKeeper* pKeeper) const override {
        reinterpret_cast<CameraPoserRail*>(pKeeper->mKeeperUser)->exeMove();
    }
};

const CameraPoserRailNrvWait NrvCameraPoserRailWait{};
const CameraPoserRailNrvMove NrvCameraPoserRailMove{};
}  // namespace

namespace al {
/**
 * Creates the camera, keeping its own copy of the placement id.
 * @param rParam The default camera parameters.
 * @param pPlayerWatcher The watcher of the players.
 * @param pPlacementId The placement id of the camera.
 */
CameraPoserRail::CameraPoserRail(const CameraPoserRailParam& rParam,
                                 const PlayerWatcher* pPlayerWatcher,
                                 const PlacementId* pPlacementId)
    : mParam(rParam), mPlayerWatcher(pPlayerWatcher) {
    mName = "Rail";
    mPlacementId = new PlacementId(*pPlacementId);
}

/**
 * Creates the rail the camera moves along and reads the move parameters of its first point.
 * @param pInfo The placement info of the camera.
 */
void CameraPoserRail::initRail(const PlacementInfo* pInfo) {
    mRailKeeper = tryCreateRailKeeper(*pInfo, "RailWithMoveParameter");
    mNerveKeeper = new NerveKeeper(this, &NrvCameraPoserRailWait, 0);

    PlacementInfo railInfo;

    if (tryGetLinksInfo(&railInfo, *pInfo, "RailWithMoveParameter")) {
        tryGetArg(&mIsReverseCoord, railInfo, "IsReverseCoord");
    }

    const PlacementInfo& pointInfo = *mRailKeeper->getRail()->getRailPoint(0);
    tryGetArg(&mSpeed, pointInfo, "Speed");
    tryGetArg(&mWaitTime, pointInfo, "WaitTime");
}

/**
 * Moves the rail rider to the rail position nearest to the top player.
 */
void CameraPoserRail::start() {
    if (mIsReverseCoord) {
        RailRider* rider = mRailKeeper->getRailRider();
        rider->setCoord(calcNearestRailCoord(mRailKeeper, mPlayerWatcher->getTopPlayerPos()));
    }
}

/**
 * Advances the rail movement and places the look at position at the rail position shifted by the
 * look at offset in view space.
 */
void CameraPoserRail::update() {
    mNerveKeeper->update();
    mLookAtPos = mRailKeeper->getRailRider()->getPosition();

    sead::LookAtCamera camera;
    makeLookAtCamera(&camera);
    camera.updateViewMatrix();

    sead::Matrix34f viewMtx = camera.getMatrix();
    sead::Matrix34f invViewMtx;
    invViewMtx.setInverse(viewMtx);

    sead::Vector3f localPos;
    localPos.setMul(viewMtx, mRailKeeper->getRailRider()->getPosition());
    localPos += mParam.lookAtOffset;
    mLookAtPos.setMul(invViewMtx, localPos);
}

/**
 * Starts moving once the wait time at a rail point has passed.
 */
void CameraPoserRail::exeWait() {
    if (mNerveKeeper->getCurrentStep() == mWaitTime) {
        mNerveKeeper->setNerve(&NrvCameraPoserRailMove);
        mRailKeeper->getRailRider()->setSpeed(mSpeed);
    }
}

/**
 * Moves along the rail and applies the move parameters of each newly reached rail point.
 */
void CameraPoserRail::exeMove() {
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
        rider->setSpeed(mSpeed);
        mNerveKeeper->setNerve(&NrvCameraPoserRailWait);
        mSectionIndex = sectionIndex;
    }
}

/**
 * Writes the pose of the camera, looking at the rail position from the configured angles.
 * @param pCamera The camera to write to.
 */
void CameraPoserRail::makeLookAtCamera(sead::LookAtCamera* pCamera) const {
    sead::Vector3f dir = sead::Vector3f::ez;
    sead::Vector3f lookAtPos = mLookAtPos;
    f32 angleV = mParam.angleV;
    f32 angleH = CameraFunction::calcAngleHFromZoneToRoot(mParam.angleH, mZoneMtx);

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
    dir.setMul(mtx, dir);

    pCamera->setPos(dir * mParam.distance + lookAtPos);
    pCamera->setAt(lookAtPos);
    pCamera->setUp(mCameraUp);
    pCamera->normalizeUp();
}

/**
 * Reads the distance, angles and look at offset of the camera.
 * @param pIter The camera parameters.
 */
void CameraPoserRail::loadParam(const ByamlIter* pIter) {
    CameraPoser::loadParam(pIter);
    pIter->tryGetFloatByKey(&mParam.distance, "Distance");
    pIter->tryGetFloatByKey(&mParam.angleV, "AngleV");
    pIter->tryGetFloatByKey(&mParam.angleH, "AngleH");

    ByamlIter offsetIter;

    if (pIter->tryGetIterByKey(&offsetIter, "LookAtOffset")) {
        offsetIter.tryGetFloatByKey(&mParam.lookAtOffset.x, "X");
        offsetIter.tryGetFloatByKey(&mParam.lookAtOffset.y, "Y");
        offsetIter.tryGetFloatByKey(&mParam.lookAtOffset.z, "Z");
    }
}

/**
 * @return The current coordinate of the rail rider.
 */
f32 CameraPoserRail::getRailCoord() const {
    return mRailKeeper->getRailRider()->getCoord();
}

/**
 * Moves the rail rider to a coordinate.
 * @param coord The new coordinate.
 */
void CameraPoserRail::setRailCoord(f32 coord) {
    mRailKeeper->getRailRider()->setCoord(coord);
}

/**
 * Creates the default rail camera parameters.
 */
CameraPoserRailParam::CameraPoserRailParam() = default;
}  // namespace al

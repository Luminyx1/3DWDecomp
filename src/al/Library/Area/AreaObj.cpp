#include "Project/AreaObj/AreaObj.hpp"

#include "Project/Base/StringUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/StageSwitch/Core/StageSwitchKeeper.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Library/Thread/Functor.hpp"
#include "Project/AreaObj/AreaShapeCube.hpp"
#include "Project/AreaObj/AreaShapeRound.hpp"

namespace al {
static AreaShape* createAreaShape(const char* pName, AreaShape* pDefault) {
    if (isEqualString(pName, "AreaCubeBase")) {
        return new AreaShapeCube(AreaShapeCube::Base);
    }
    if (isEqualString(pName, "AreaCubeCenter")) {
        return new AreaShapeCube(AreaShapeCube::Center);
    }
    if (isEqualString(pName, "AreaSphere")) {
        return new AreaShapeOval();
    }
    if (isEqualString(pName, "AreaCylinder")) {
        return new AreaShapeCylinder();
    }
    return pDefault;
}

/**
 * Constructs an area object with the given name.
 * @param pName name of the area
 */
AreaObj::AreaObj(const char* pName) : mName(pName) {}

/**
 * Initializes the area from its placement info.
 * @param rInfo area init info
 */
void AreaObj::init(const AreaInitInfo& rInfo) {
    using AreaObjFunctor = FunctorV0M<AreaObj*, void (AreaObj::*)()>;

    mPlacementInfo = new PlacementInfo(rInfo.mPlacementInfo);
    tryGetZoneID(&mZoneID, rInfo.mPlacementInfo);
    tryGetArg(&mScenarioID, rInfo.mPlacementInfo, "ScenarioID");
    tryGetMatrixTR(&_28, *mPlacementInfo);

    const char* modelName = nullptr;
    alPlacementFunction::tryGetModelName(&modelName, *mPlacementInfo);
    if (isEqualString(modelName, "AreaCubeBase")) {
        mShape = new AreaShapeCube(AreaShapeCube::Base);
    } else if (isEqualString(modelName, "AreaCubeCenter")) {
        mShape = new AreaShapeCube(AreaShapeCube::Center);
    } else if (isEqualString(modelName, "AreaSphere")) {
        mShape = new AreaShapeOval();
    } else if (isEqualString(modelName, "AreaCylinder")) {
        mShape = new AreaShapeCylinder();
    }
    mShape->setBaseMtxPtr(&_28);

    tryGetArg(&mPriority, *mPlacementInfo, "Priority");
    tryGetArg(&mIsSpawnEntranceCamera, *mPlacementInfo, "IsSpawnEntranceCamera");
    tryGetArg(&mIsNoSinkOcean, *mPlacementInfo, "IsNoSinkOcean");
    tryGetArg(&mIsCodeLink, *mPlacementInfo, "IsCodeLink");
    tryGetArg(&mIsDisasterCameraOn, *mPlacementInfo, "IsDisasterCameraOn");
    tryGetArg(&mIsPlessieCameraOn, *mPlacementInfo, "IsPlessieCameraOn");
    tryGetArg(&mIsDisablePushControl, *mPlacementInfo, "IsDisablePushControl");
    tryGetArg(&mIsSnapToCamera, *mPlacementInfo, "IsSnapToCamera");
    tryGetArg(&mIsUIMapTriggered, *mPlacementInfo, "IsUIMapTriggered");
    tryGetArg(&mIsOnGroundOnly, *mPlacementInfo, "IsOnGroundOnly");
    tryGetArg(&mIsOneWayGroundOnly, *mPlacementInfo, "IsOneWayGroundOnly");
    tryGetArg(&mIsPlessieTunnel, *mPlacementInfo, "IsPlessieTunnel");
    tryGetArg(&mIsPlessieChaseV2SpecialCamera, *mPlacementInfo, "IsPlessieChaseV2SpecialCamera");
    tryGetArg(&mIsPlessieRideOnly, *mPlacementInfo, "IsPlessieRideOnly");
    if (mIsPlessieCameraOn || mIsUIMapTriggered) {
        mIsValid = false;
    }

    sead::Vector3f scale = {1.0f, 1.0f, 1.0f};
    tryGetScale(&scale, *mPlacementInfo);
    mShape->setScale(scale);

    initStageSwitch(this, rInfo.mSwitchDirector, rInfo.mPlacementInfo);
    bool isListenAppear = listenStageSwitchOnOffAppear(
        this, AreaObjFunctor(this, &AreaObj::validate), AreaObjFunctor(this, &AreaObj::invalidate));
    if (isListenAppear) {
        invalidate();
    }
    if (listenStageSwitchOnKill(this, AreaObjFunctor(this, &AreaObj::invalidate)) &&
        !isListenAppear) {
        validate();
    }
    if (listenStageSwitchOnOff(this, "SwitchEnableOn", AreaObjFunctor(this, &AreaObj::enable),
                               AreaObjFunctor(this, &AreaObj::disable))) {
        disable();
    }
}

/**
 * Creates the stage switch keeper.
 */
void AreaObj::initStageSwitchKeeper() {
    mSwitchKeeper = new StageSwitchKeeper();
}

/**
 * Checks whether a position is inside the area, if the area is active.
 * @param rPos position to check
 * @return true if the area is active and contains the position
 */
bool AreaObj::isInVolume(const sead::Vector3f& rPos) const {
    if (!mIsValid || mIsDisabled || !_66) {
        return false;
    }
    return mShape->isInVolume(rPos);
}

/**
 * Checks whether a position is inside the area shape, ignoring the area state.
 * @param rPos position to check
 * @return true if the shape contains the position
 */
bool AreaObj::isInVolumeCheck(const sead::Vector3f& rPos) const {
    return mShape->isInVolume(rPos);
}

/**
 * Checks a line segment against the area shape, if the area is active.
 * @param rStart start of the segment
 * @param rEnd end of the segment
 * @param pHitPos output hit position
 * @param pNormal output hit normal
 * @return true if the area is active and the segment hits the shape
 */
bool AreaObj::isInVolume(const sead::Vector3f& rStart, const sead::Vector3f& rEnd,
                         sead::Vector3f* pHitPos, sead::Vector3f* pNormal) {
    if (!mIsValid || mIsDisabled || !_66) {
        return false;
    }
    return mShape->checkArrowCollision(pHitPos, pNormal, rStart, rEnd);
}

/**
 * Sets the start position of the area.
 * @param rPos start position
 */
void AreaObj::setStartPos(sead::Vector3f& rPos) {
    mIsStartPosSet = true;
    mStartPos = rPos;
}

/**
 * Gets the start position of the area.
 * @param pPos output start position
 * @return true if a start position was set
 */
bool AreaObj::getStartPos(sead::Vector3f* pPos) {
    *pPos = mStartPos;
    return mIsStartPosSet;
}
}  // namespace al

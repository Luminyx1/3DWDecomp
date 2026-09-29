#include "Project/AreaObj/AreaObj.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/StageSwitch/Core/StageSwitchKeeper.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Library/Thread/Functor.hpp"
#include "Project/AreaObj/AreaShapeCube.hpp"
#include "Project/AreaObj/AreaShapeRound.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
    /**
     * @brief Constructs an area that is valid and enabled.
     * @param pName The name of the area.
     */
    AreaObj::AreaObj(const char* pName) : mName(pName) {}

    /**
     * @brief Initializes the area's placement, shape, parameters and switches.
     * @param rInfo The area's init info.
     */
    void AreaObj::init(const AreaInitInfo& rInfo) {
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

        sead::Vector3f scale(1.0f, 1.0f, 1.0f);
        tryGetScale(&scale, *mPlacementInfo);
        mShape->setScale(scale);

        initStageSwitch(this, rInfo.mSwitchDirector, rInfo.mPlacementInfo);

        bool isListenAppear = listenStageSwitchOnOffAppear(this, FunctorV0M<AreaObj*, void (AreaObj::*)()>(this, &AreaObj::validate),
                                                           FunctorV0M<AreaObj*, void (AreaObj::*)()>(this, &AreaObj::invalidate));
        if (isListenAppear) {
            mIsValid = false;
        }

        bool isListenKill = listenStageSwitchOnKill(this, FunctorV0M<AreaObj*, void (AreaObj::*)()>(this, &AreaObj::invalidate));
        if (!isListenAppear && isListenKill) {
            mIsValid = true;
        }

        if (listenStageSwitchOnOff(this, "SwitchEnableOn", FunctorV0M<AreaObj*, void (AreaObj::*)()>(this, &AreaObj::enable),
                                   FunctorV0M<AreaObj*, void (AreaObj::*)()>(this, &AreaObj::disable))) {
            mIsDisabled = true;
        }
    }

    /** @brief Makes the area invalid. */
    void AreaObj::invalidate() {
        mIsValid = false;
    }

    /** @brief Makes the area valid. */
    void AreaObj::validate() {
        mIsValid = true;
    }

    /** @brief Enables the area. */
    void AreaObj::enable() {
        mIsDisabled = false;
    }

    /** @brief Disables the area. */
    void AreaObj::disable() {
        mIsDisabled = true;
    }

    /** @brief Creates the area's stage switch keeper. */
    void AreaObj::initStageSwitchKeeper() {
        mSwitchKeeper = new StageSwitchKeeper();
    }

    /**
     * @brief Checks whether a position is inside the area, if the area is active.
     * @param rPos The position to check.
     * @return Whether the area is active and contains the position.
     */
    bool AreaObj::isInVolume(const sead::Vector3f& rPos) const {
        if (!mIsValid || mIsDisabled || !_66) {
            return false;
        }

        return mShape->isInVolume(rPos);
    }

    /**
     * @brief Checks whether a position is inside the area's shape.
     * @param rPos The position to check.
     * @return Whether the shape contains the position.
     */
    bool AreaObj::isInVolumeCheck(const sead::Vector3f& rPos) const {
        return mShape->isInVolume(rPos);
    }

    /**
     * @brief Checks whether a line segment enters the area, if the area is active.
     * @param rStart The start of the segment.
     * @param rEnd The end of the segment.
     * @param pHitPos Receives the entry position.
     * @param pHitNormal Receives the normal at the entry position.
     * @return Whether the area is active and the segment enters it.
     */
    bool AreaObj::isInVolume(const sead::Vector3f& rStart, const sead::Vector3f& rEnd, sead::Vector3f* pHitPos,
                             sead::Vector3f* pHitNormal) {
        if (!mIsValid || mIsDisabled || !_66) {
            return false;
        }

        return mShape->checkArrowCollision(pHitPos, pHitNormal, rStart, rEnd);
    }

    /**
     * @brief Sets the start position of the area.
     * @param rPos The start position.
     */
    void AreaObj::setStartPos(sead::Vector3f& rPos) {
        _75 = true;
        mStartPos.x = rPos.x;
        mStartPos.y = rPos.y;
        mStartPos.z = rPos.z;
    }

    /**
     * @brief Gets the start position of the area.
     * @param pPos Receives the start position.
     * @return Whether a start position has been set.
     */
    bool AreaObj::getStartPos(sead::Vector3f* pPos) {
        pPos->x = mStartPos.x;
        pPos->y = mStartPos.y;
        pPos->z = mStartPos.z;
        return _75;
    }
};

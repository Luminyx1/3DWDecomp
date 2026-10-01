#include "Library/Camera/CameraPoseUpdater.hpp"

#include <math/seadMathCalcCommon.h>
#include <nn/oe.h>

#include "Library/Camera/CameraParamTransfer.hpp"
#include "Library/Camera/CameraPoseInfo.hpp"
#include "Library/Camera/CameraPoserFlag.hpp"
#include "Library/Camera/CameraPoserFunction.hpp"
#include "Library/Camera/CameraPoser_RS.hpp"
#include "Library/Camera/CameraStartInfo.hpp"
#include "Library/Camera/CameraTicket.hpp"
#include "Library/Camera/CameraUtil.hpp"
#include "Library/Camera/CameraViewFlag.hpp"
#include "Library/Camera/SceneCameraCtrl.hpp"
#include "Library/Clipping/ClippingDirectorBase.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Play/Camera/CameraVerticalAbsorber.hpp"
#include "Library/Projection/OrthoProjectionInfo.hpp"
#include "Library/Projection/Projection.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"
#include "Project/Camera/CameraAngleSwingInfo.hpp"
#include "Project/Camera/Holder/CameraInterpole_RS.hpp"
#include "Project/Camera/Holder/CameraShaker_RS.hpp"
#include "Project/Camera/Holder/CameraStartParamCtrl.hpp"
#include "Project/Camera/Holder/CameraStopJudge.hpp"
#include "Project/Camera/Holder/CameraSwitcher_RS.hpp"
#include "Project/Camera/Info/CameraViewInfo.hpp"
#include "Project/Camera/Info/SceneCameraInfo.hpp"

namespace {
using namespace al;

NERVE_DECL(CameraPoseUpdater, Deactive)
NERVE_DECL(CameraPoseUpdater, Pause)
NERVE_DECL(CameraPoseUpdater, Stop)
NERVE_DECL(CameraPoseUpdater, SnapShotNoUpdate)

class CameraPoseUpdaterNrvSnapShot : public al::Nerve {
public:
    void execute(al::NerveKeeper* pKeeper) const override {
        pKeeper->getParent<CameraPoseUpdater>()->exeSnapShot();
    }

    void executeOnEnd(al::NerveKeeper* pKeeper) const override {
        pKeeper->getParent<CameraPoseUpdater>()->endSnapShot();
    }
};

NERVE_DECL(CameraPoseUpdater, Active)

NERVES_MAKE_NOSTRUCT(CameraPoseUpdater, Deactive, Pause, Stop, SnapShotNoUpdate, SnapShot, Active)
}  // namespace

namespace al {

/**
 * Creates the pose updater of one camera view.
 * @param pSceneCameraInfo Scene camera info the view is registered to.
 * @param viewIndex Index of the view.
 */
CameraPoseUpdater::CameraPoseUpdater(SceneCameraInfo* pSceneCameraInfo, s32 viewIndex)
    : NerveExecutor("カメラの姿勢更新"), mSceneCameraInfo(pSceneCameraInfo),
      mIsMainView(viewIndex == 0), mViewIndex(viewIndex) {
    mViewFlag = new CameraViewFlag();
    mProjection = new Projection();
    mOrthoProjectionInfo = new OrthoProjectionInfo();
    mViewInfo = new CameraViewInfo(mViewIndex, mLookAtCamera, *mProjection, *mViewFlag,
                                   *mOrthoProjectionInfo);
    if (!mIsMainView) {
        mViewInfo->setValid(false);
    }

    pSceneCameraInfo->initViewInfo(mViewInfo);
    initNerve(&NrvCameraPoseUpdaterDeactive, 0);
}

/**
 * Sets the clipping director notified on camera switches.
 * @param pDirector Clipping director.
 */
void CameraPoseUpdater::setClippingDirector(ClippingDirectorBase* pDirector) {
    mClippingDirector = pDirector;
}

/**
 * Resets the screenshot orientation and destroys the updater.
 */
CameraPoseUpdater::~CameraPoseUpdater() {
    nn::oe::SetAlbumImageOrientation(nn::album::ImageOrientation_None);
}

/**
 * Returns the projection of this view.
 * @return Projection.
 */
Projection* CameraPoseUpdater::getProjection() {
    return mProjection;
}

/**
 * Initializes the updater with its helpers and creates the shaker and interpolation.
 * @param pParamTransfer Parameter transfer between posers.
 * @param pStopJudge Camera stop judge.
 * @param pStartParamCtrl Camera start parameter control.
 */
void CameraPoseUpdater::init(const CameraParamTransfer* pParamTransfer,
                             const CameraStopJudge* pStopJudge,
                             CameraStartParamCtrl* pStartParamCtrl) {
    mParamTransfer = pParamTransfer;
    mStopJudge = pStopJudge;
    mStartParamCtrl = pStartParamCtrl;
    mShaker = new CameraShaker_RS();
    mInterpole = new CameraInterpole_RS();
    mLookAtCamera.updateViewMatrix();
    mProjection->setProj(getNearClipDistance(), mFarClipDistance,
                         sead::Mathf::deg2rad(mFovyDegree), mAspect);
}

/**
 * Returns the near clip distance of the current camera or the default one.
 * @return Near clip distance.
 */
f32 CameraPoseUpdater::getNearClipDistance() const {
    return (mTicket != nullptr) && mTicket->getPoser()->getNearClipDistance() > 0.0f ?
               mTicket->getPoser()->getNearClipDistance() :
               mNearClipDistance;
}

/**
 * Updates the camera pose, shake and projection of this view.
 * @param isPaused Whether the scene is paused.
 */
void CameraPoseUpdater::update(bool isPaused) {
    mViewFlag->resetAllFlag();
    *mOrthoProjectionInfo = {};
    if (!isNerve(this, &NrvCameraPoseUpdaterPause) && !isNerve(this, &NrvCameraPoseUpdaterStop) &&
        !isNerve(this, &NrvCameraPoseUpdaterSnapShot) &&
        !isNerve(this, &NrvCameraPoseUpdaterSnapShotNoUpdate)) {
        trySwitchCamera();
    }

    updateNerve();
    bool isMainView = mIsMainView;
    mViewInfo->setValid(isMainView);

    mShaker->update(mSceneCameraViewCtrl->getShakeName(), isPaused);
    mProjection->setOffset(mShaker->getOffset());
    {
        sead::Vector3f up = mLookAtCamera.getUp();
        rotateVectorDegreeZ(&up, mShaker->getRoll());
        mLookAtCamera.setUp(up);
        mLookAtCamera.normalizeUp();
    }

    mSceneCameraViewCtrl->setShakeName(nullptr);

    if (mAreaObjDirector != nullptr) {
        sead::Vector3f pos = mLookAtCamera.getPos();
        pos.y += -50.0f;

        if (isInWaterArea(this, pos)) {
            f32 distance;

            if (calcWaterDistanceCheck(this, pos, 1000.0f, &distance)) {
                pos.y = distance + 50.0f;
                mLookAtCamera.setPos(pos);
            }
        }
    }

    mLookAtCamera.updateViewMatrix();

    f32 fovy = mFovyDegree;

    if (mPauseCameraCtrl != nullptr && mPauseCameraCtrl->isCameraPause()) {
        fovy = mPauseCameraCtrl->getFovyDegree();
    }

    mProjection->setProj(getNearClipDistance(), mFarClipDistance, sead::Mathf::deg2rad(fovy),
                         mAspect);
    mProjection->calcMtx();

    if (mPauseCameraCtrl == nullptr || !mPauseCameraCtrl->isCameraPause()) {
        mViewInfo->setFirstCalc(false);
    }
}

/**
 * Switches to the next camera requested by the switcher.
 * @return Whether the nerve was changed.
 */
bool CameraPoseUpdater::trySwitchCamera() {
    if (mSwitcher == nullptr) {
        return false;
    }

    mSwitcher->update();

    if (!mSwitcher->isChanged()) {
        return false;
    }

    if (!mSwitcher->isExistNextCamera()) {
        mTicket = nullptr;

        if (!isNerve(this, &NrvCameraPoseUpdaterDeactive)) {
            setNerve(this, &NrvCameraPoseUpdaterDeactive);
            return true;
        }

        return false;
    }

    bool isChangedNerve = false;

    if (!isNerve(this, &NrvCameraPoseUpdaterActive)) {
        setNerve(this, &NrvCameraPoseUpdaterActive);
        isChangedNerve = true;
    }

    CameraTicket* prevTicket = mTicket;
    mTicket = mSwitcher->getNextCamera();

    CameraStartInfo startInfo;

    if (prevTicket != nullptr) {
        startInfo.prePriorityType = static_cast<CameraTicket::Priority>(prevTicket->getPriority());
        startInfo.preCameraName = prevTicket->getPoser()->getName();
        startInfo.isInvalidCollidePreCamera =
            prevTicket->getPoser()->getPoserFlag()->isInvalidCollider;
        startInfo.isInvalidKeepPreCameraDistance =
            prevTicket->getPoser()->getPoserFlag()->isInvalidKeepDistanceNextCamera;
        startInfo.isInvalidKeepPreCameraDistanceIfNoCollide =
            prevTicket->getPoser()->getPoserFlag()->isInvalidKeepDistanceNextCameraIfNoCollide;
        startInfo.isValidResetPreCameraPose = prevTicket->getPoser()->getPoserFlag()->_3;

        if (mSwitcher->isNextKeepPose()) {
            startInfo.isValidKeepPreSelfCameraPose = false;
        } else if (prevTicket->getPoser()->getPoserFlag()->isValidKeepPreSelfPoseNextCamera()) {
            startInfo.isValidKeepPreSelfCameraPose = true;
        }

        CameraVerticalAbsorber* absorber = prevTicket->getPoser()->getCameraVerticalAbsorber();

        if (absorber != nullptr && absorber->isAbsorbing()) {
            startInfo._25 = true;
        }

        if (prevTicket->getPoser()->getAngleSwingInfo() != nullptr) {
            startInfo.preCameraSwingAngleH =
                prevTicket->getPoser()->getAngleSwingInfo()->currentAngle.x;
            startInfo.preCameraSwingAngleV =
                prevTicket->getPoser()->getAngleSwingInfo()->currentAngle.y;
            startInfo.preCameraMaxSwingAngleH =
                prevTicket->getPoser()->getAngleSwingInfo()->maxSwingDegreeH;
            startInfo.preCameraMaxSwingAngleV =
                prevTicket->getPoser()->getAngleSwingInfo()->maxSwingDegreeV;
        }

        if (mSwitcher->isSetNextPoseInfo()) {
            sead::Vector3f dir = mSwitcher->getNextPoseInfo()->pos - mSwitcher->getNextPoseInfo()->at;
            normalize(&dir);
            startInfo.isExistNextPoseByPreCamera = true;
            startInfo.nextAngleHByPreCamera =
                sead::Mathf::rad2deg(sead::Mathf::atan2(dir.x, dir.z));
            startInfo.nextAngleVByPreCamera = sead::Mathf::rad2deg(sead::Mathf::asin(dir.y));
        }

        mStartParamCtrl->tryApplyParam(&startInfo);
    }

    mTicket->getPoser()->setViewInfo(mViewInfo);
    mTicket->getPoser()->appear(startInfo);

    if (prevTicket != nullptr) {
        mInterpole->start(mTicket, mFovyDegree, mSwitcher->getNextInterpoleStep());
        mParamTransfer->tryTransferParam(prevTicket->getPoser(), mTicket->getPoser());
    } else {
        mInterpole->setTicket(mTicket);
    }

    if (mClippingDirector != nullptr && mSwitcher->isChanged() && mInterpole->getStep() <= 0) {
        mClippingDirector->resetClippingDistanceStates();
    }

    return isChangedNerve;
}

/**
 * Returns whether the camera interpolation is active.
 * @return Whether the interpolation is active.
 */
bool CameraPoseUpdater::isActiveInterpole() const {
    return mInterpole->isActive();
}

/**
 * Starts interpolating to the current camera.
 * @param step Interpolation frames.
 */
void CameraPoseUpdater::startInterpole(s32 step) {
    mInterpole->start(mTicket, mFovyDegree, step);
}

/**
 * Requests to cancel the camera interpolation.
 */
void CameraPoseUpdater::requestCancelInterpole() {
    mInterpole->requestCancel();
}

/**
 * Calculates the pose of the current camera without interpolation.
 * @param pCamera Receives the camera pose.
 * @return Whether a camera is active.
 */
bool CameraPoseUpdater::calcCameraPoseWithoutInterpole(sead::LookAtCamera* pCamera) const {
    if (mTicket == nullptr) {
        return false;
    }

    mTicket->getPoser()->calcCameraPose(pCamera);
    return true;
}

/**
 * Starts the snapshot mode.
 * @param isLock Whether the camera is locked in snapshot mode.
 */
void CameraPoseUpdater::startSnapShotMode(bool isLock) {
    if (mTicket == nullptr) {
        return;
    }

    if (isNerve(this, &NrvCameraPoseUpdaterSnapShot) ||
        isNerve(this, &NrvCameraPoseUpdaterSnapShotNoUpdate)) {
        return;
    }

    if (isLock || mInterpole->isActive() || isNerve(this, &NrvCameraPoseUpdaterStop)) {
        setNerve(this, &NrvCameraPoseUpdaterSnapShotNoUpdate);
        return;
    }

    sead::Vector3f trans = {0.0f, 0.0f, 0.0f};
    alCameraPoserFunction::calcTargetTrans(&trans, mTicket->getPoser());
    AreaObj* areaObj = tryFindAreaObj(mTicket->getPoser(), "SnapShotInvalidCtrlArea", trans);

    if (areaObj != nullptr) {
        bool isValidCtrl = false;

        if (!tryGetAreaObjArg(&isValidCtrl, areaObj, "IsValidCtrl") || !isValidCtrl) {
            setNerve(this, &NrvCameraPoseUpdaterSnapShotNoUpdate);
            return;
        }
    }

    mTicket->getPoser()->startSnapShotModeCore();
    setNerve(this, &NrvCameraPoseUpdaterSnapShot);
}

/**
 * Enables or disables rolling the camera in snapshot mode.
 * @param isEnable Whether rolling is enabled.
 */
void CameraPoseUpdater::enableSnapShotRoll(bool isEnable) {
    if (mTicket != nullptr) {
        mTicket->getPoser()->enableSnapShotRoll(isEnable);
    }
}

/**
 * Ends the snapshot mode.
 */
void CameraPoseUpdater::endSnapShotMode() {
    if (isNerve(this, &NrvCameraPoseUpdaterSnapShot)) {
        mTicket->getPoser()->endSnapShotModeCore();
    } else if (!isNerve(this, &NrvCameraPoseUpdaterSnapShotNoUpdate)) {
        return;
    }

    if (mStopJudge->isStop()) {
        setNerve(this, &NrvCameraPoseUpdaterStop);
    } else {
        setNerve(this, &NrvCameraPoseUpdaterActive);
    }
}

/**
 * Returns whether the snapshot image is rotated by 90 degrees.
 * @return Whether the orientation is Rotate90.
 */
bool CameraPoseUpdater::isSnapShotOrientationRotate90() const {
    return mSnapShotOrientation == nn::album::ImageOrientation_Rotate90;
}

/**
 * Returns whether the snapshot image is rotated by 270 degrees.
 * @return Whether the orientation is Rotate270.
 */
bool CameraPoseUpdater::isSnapShotOrientationRotate270() const {
    return mSnapShotOrientation == nn::album::ImageOrientation_Rotate270;
}

/**
 * Updates the view from the current camera.
 */
void CameraPoseUpdater::exeActive() {
    if (isFirstStep(this)) {
        mIsMainView = true;
    }

    if (mPauseCameraCtrl != nullptr && mPauseCameraCtrl->isCameraPause() &&
        !isNerve(this, &NrvCameraPoseUpdaterPause)) {
        setNerve(this, &NrvCameraPoseUpdaterPause);
        return;
    }

    if (mStopJudge->isStop()) {
        setNerve(this, &NrvCameraPoseUpdaterStop);
        return;
    }

    sead::LookAtCamera camera;
    OrthoProjectionInfo* orthoInfo = mOrthoProjectionInfo;
    CameraPoser_RS* poser = mTicket->getPoser();
    poser->movement();
    poser->calcCameraPose(&camera);
    poser->tryCalcOrthoProjectionInfo(orthoInfo);
    mFovyDegree = poser->getFovyDegree();

    if (!mTicket->getPoser()->is141()) {
        mInterpole->update(camera);
    }

    mInterpole->makeLookAtCamera(&camera);

    if (mInterpole->isActive()) {
        mFovyDegree = mInterpole->getFovyDegree();
        mViewInfo->setActiveInterpole(true);
    } else {
        mViewInfo->setActiveInterpole(false);
    }

    mLookAtCamera.setPos(camera.getPos());
    mLookAtCamera.setAt(camera.getAt());
    mLookAtCamera.setUp(camera.getUp());
    mLookAtCamera.normalizeUp();

    bool isInvalidCameraBlur = mTicket->getPoser()->getPoserFlag()->isInvalidCameraBlur;
    mViewFlag->setInvalidCameraBlur(isInvalidCameraBlur);
}

/**
 * Sets the camera used when no other camera is active.
 * @param pTicket Default camera ticket.
 */
void CameraPoseUpdater::setDefaultTicket(CameraTicket* pTicket) {
    if (pTicket != nullptr) {
        mDefaultTicket = pTicket;
        mFarClipDistance = pTicket->getPoser()->getFarClipDistance();
    }
}

/**
 * Copies the pose of the main view while this view has no camera.
 */
void CameraPoseUpdater::exeDeactive() {
    if (isFirstStep(this)) {
        mIsMainView = false;
    }

    if (mPauseCameraCtrl != nullptr && mPauseCameraCtrl->isCameraPause() &&
        !isNerve(this, &NrvCameraPoseUpdaterPause)) {
        setNerve(this, &NrvCameraPoseUpdaterPause);
        return;
    }

    const CameraViewInfo* viewInfo = mSceneCameraInfo->getViewAt(0);
    mLookAtCamera.setPos(viewInfo->getLookAtCam().getPos());
    mLookAtCamera.setAt(viewInfo->getLookAtCam().getAt());
    mLookAtCamera.setUp(viewInfo->getLookAtCam().getUp());
    mLookAtCamera.normalizeUp();
    mFovyDegree = sead::Mathf::rad2deg(viewInfo->getProjection().getFovy());
}

/**
 * Keeps the camera still until the stop judge releases it.
 */
void CameraPoseUpdater::exeStop() {
    if (mPauseCameraCtrl != nullptr && mPauseCameraCtrl->isCameraPause() &&
        !isNerve(this, &NrvCameraPoseUpdaterPause)) {
        setNerve(this, &NrvCameraPoseUpdaterPause);
        return;
    }

    if (mStopJudge->isStop()) {
        return;
    }

    if (mTicket != nullptr) {
        startInterpole(60);
        setNerve(this, &NrvCameraPoseUpdaterActive);
    } else {
        setNerve(this, &NrvCameraPoseUpdaterDeactive);
    }
}

/**
 * Keeps the camera paused until the pause ends.
 */
void CameraPoseUpdater::exePause() {
    if (mPauseCameraCtrl->isCameraPause()) {
        return;
    }

    if (mStopJudge->isStop()) {
        setNerve(this, &NrvCameraPoseUpdaterStop);
    } else if (mTicket != nullptr) {
        setNerve(this, &NrvCameraPoseUpdaterActive);
    } else {
        setNerve(this, &NrvCameraPoseUpdaterDeactive);
    }
}

/**
 * Updates the camera in snapshot mode and the screenshot orientation.
 */
void CameraPoseUpdater::exeSnapShot() {
    sead::LookAtCamera* camera = &mLookAtCamera;
    OrthoProjectionInfo* orthoInfo = mOrthoProjectionInfo;
    CameraPoser_RS* poser = mTicket->getPoser();
    poser->movement();
    poser->movement();
    poser->calcCameraPose(camera);
    poser->tryCalcOrthoProjectionInfo(orthoInfo);
    mFovyDegree = poser->getFovyDegree();

    s32 orientation = calcSnapShotImageOrientation(mLookAtCamera, 60.0f);
    nn::oe::SetAlbumImageOrientation(static_cast<nn::album::ImageOrientation>(orientation));
    mSnapShotOrientation = orientation;
}

/**
 * Resets the screenshot orientation at the end of the snapshot mode.
 */
void CameraPoseUpdater::endSnapShot() {
    nn::oe::SetAlbumImageOrientation(nn::album::ImageOrientation_None);
    mSnapShotOrientation = nn::album::ImageOrientation_None;
}

/**
 * Keeps the camera still in snapshot mode.
 */
void CameraPoseUpdater::exeSnapShotNoUpdate() {}

/**
 * Returns whether the current camera has the given priority.
 * @param priority Camera priority.
 * @return Whether the current camera has the priority.
 */
bool CameraPoseUpdater::isCurrentCameraPriority(s32 priority) const {
    return (mTicket != nullptr) && mTicket->getPriority() == priority;
}

/**
 * Returns whether the current camera forbids changing to the subjective camera.
 * @return Whether changing to the subjective camera is invalid.
 */
bool CameraPoseUpdater::isInvalidChangeSubjectiveCamera() const {
    return (mTicket != nullptr) && mTicket->getPoser()->getPoserFlag()->isInvalidChangeSubjective;
}

/**
 * Returns whether the current camera is zooming.
 * @return Whether the current camera is zooming.
 */
bool CameraPoseUpdater::isCurrentCameraZooming() const {
    return (mTicket != nullptr) && mTicket->getPoser()->isZooming();
}

/**
 * Returns whether the current camera can be rotated with the stick.
 * @return Whether rotating by pad is enabled.
 */
bool CameraPoseUpdater::isCurrentCameraEnableRotateByPad() const {
    return (mTicket != nullptr) && mTicket->getPoser()->isEnableRotateByPad();
}

/**
 * Returns whether the current camera ticket has the disaster flag set.
 * @return Whether the disaster flag is set.
 */
bool CameraPoseUpdater::isCurrentCameraDisasterOn() const {
    return (mTicket != nullptr) && mTicket->is15();
}

/**
 * Passes an object request to the current camera.
 * @param rInfo Request info.
 * @return Whether the request was accepted.
 */
bool CameraPoseUpdater::tryReceiveCameraRequestFromObject(const CameraObjectRequestInfo& rInfo) {
    return (mTicket != nullptr) && mTicket->getPoser()->receiveRequestFromObjectCore(rInfo);
}

/**
 * Requests the current camera to turn to a direction.
 * @param pInfo Turn info.
 * @return Whether the request was accepted.
 */
bool CameraPoseUpdater::tryRequestCameraTurnToDirection(const CameraTurnInfo* pInfo) {
    return (mTicket != nullptr) && mTicket->getPoser()->requestTurnToDirection(pInfo);
}

}  // namespace al

#include "Library/Play/Camera/CameraPoserZoomParam.hpp"

#include <gfx/seadCamera.h>
#include <math/seadMathCalcCommon.h>
#include <math/seadMatrix.h>
#include <math/seadQuat.h>

#include "Library/Controller/InputFunction.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/Nerve.hpp"
#include "Library/Nerve/NerveKeeper.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Camera/PlayerWatcher.hpp"
#include "Library/Play/Placement/PlacementId.hpp"
#include "Library/Se/Function/SeFunction.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Camera/ControlAngleParam.hpp"

namespace {
using namespace al;

class CameraPoserZoomNrvWait : public al::Nerve {
public:
    void execute(al::NerveKeeper* pKeeper) const override {
        reinterpret_cast<CameraPoserZoom*>(pKeeper->mKeeperUser)->exeWait();
    }
};

class CameraPoserZoomNrvInterpolate : public al::Nerve {
public:
    void execute(al::NerveKeeper* pKeeper) const override {
        reinterpret_cast<CameraPoserZoom*>(pKeeper->mKeeperUser)->exeInterpolate();
    }
};

class CameraPoserZoomNrvInvalid : public al::Nerve {
public:
    void execute(al::NerveKeeper* pKeeper) const override {
        reinterpret_cast<CameraPoserZoom*>(pKeeper->mKeeperUser)->exeInvalid();
    }
};

// Non-const so that the three nerves are merged into one block, as in the original binary.
CameraPoserZoomNrvWait NrvCameraPoserZoomWait;
CameraPoserZoomNrvInterpolate NrvCameraPoserZoomInterpolate;
CameraPoserZoomNrvInvalid NrvCameraPoserZoomInvalid;

const CameraPoserZoomParam sDefaultParamNear(2400.0f, 27.0f, {0.0f, 100.0f, 0.0f});
const CameraPoserZoomParam sDefaultParamNormal(3300.0f, 31.0f, {0.0f, 100.0f, 0.0f});
const CameraPoserZoomParam sDefaultParamFar(5500.0f, 45.0f, {0.0f, 100.0f, 0.0f});

/**
 * Reads the vertical tilt of the right stick of a controller.
 * @param port Controller port.
 * @return The vertical stick tilt, or zero if the stick is unavailable.
 */
inline f32 getRightStickY(s32 port) {
    if (isPadEnableRightStick(port)) {
        return getRightStick(port).y;
    }

    // The original queries the main controller port here without using it.
    getMainControllerPort();
    return 0.0f;
}

/**
 * Reads the zoom direction from the vertical tilt of the right stick.
 * @param pZoomDir Receives the zoom level change.
 * @param port Controller port.
 * @param pIsReverse Whether the zoom direction is reversed.
 * @return Whether the stick is tilted far enough.
 */
inline bool tryGetZoomInput(s32* pZoomDir, s32 port, const bool* pIsReverse) {
    f32 stickY = getRightStickY(port);

    if (sead::Mathf::abs(stickY) < 0.3f) {
        return false;
    }

    s32 zoomDir = stickY > 0.0f ? -1 : 1;
    *pZoomDir = *pIsReverse ? -zoomDir : zoomDir;
    return true;
}

/**
 * @return Whether the right stick was pressed on any controller.
 */
inline bool isTriggerPressRightStickAny() {
    if (isPadTriggerPressRightStick(getMainControllerPort())) {
        return true;
    }

    for (s32 port = 1; port < 4; port++) {
        if (isPadEnableRightStick(port) && isPadTriggerPressRightStick(port)) {
            return true;
        }
    }

    return false;
}

/**
 * @return Whether L was pressed on any controller that is not a single Joy-Con.
 */
inline bool isTriggerLAny() {
    if (!isPadTypeJoySingle(getMainControllerPort()) && isPadTriggerL(getMainControllerPort())) {
        return true;
    }

    for (s32 port = 1; port < 5; port++) {
        if (!isPadTypeJoySingle(port) && isPadTriggerL(port)) {
            return true;
        }
    }

    return false;
}
}  // namespace

namespace al {
/**
 * Reads the parameters of one zoom level.
 * @param pPoser The zoom camera.
 * @param level Zoom level.
 * @param pIter The camera parameters.
 * @param pKey Key of the zoom level parameters.
 */
inline void tryLoadZoomParam(CameraPoserZoom* pPoser, s32 level, const ByamlIter* pIter,
                             const char* pKey) {
    ByamlIter iter;

    if (pIter->tryGetIterByKey(&iter, pKey)) {
        CameraPoserZoomParam* param = pPoser->mZoomParams[level];
        iter.tryGetFloatByKey(&param->mDistance, "Distance");
        iter.tryGetFloatByKey(&param->mAngleV, "AngleV");
        tryGetByamlV3f(&param->mOffsetLookAt, iter, "OffsetLookAt");
    }
}

/**
 * Creates the zoom parameters of the normal zoom level.
 */
CameraPoserZoomParam::CameraPoserZoomParam()
    : mDistance(3300.0f), mAngleV(31.0f), mOffsetLookAt(0.0f, 100.0f, 0.0f) {}

/**
 * Creates zoom parameters.
 * @param distance Distance from the look at position.
 * @param angleV Vertical angle in degrees.
 * @param offsetLookAt Offset of the look at position from the player.
 */
CameraPoserZoomParam::CameraPoserZoomParam(f32 distance, f32 angleV, sead::Vector3f offsetLookAt)
    : mDistance(distance), mAngleV(angleV), mOffsetLookAt(offsetLookAt) {}

/**
 * Creates the zoom camera with near, normal and far zoom levels.
 * @param pPlayerWatcher The watcher of the players.
 * @param pPlacementId The placement id of the camera.
 * @param pAudioKeeper Audio keeper used for the zoom sounds.
 * @param pIsReverseZoomInput Whether the stick zoom direction is reversed.
 */
CameraPoserZoom::CameraPoserZoom(const PlayerWatcher* pPlayerWatcher,
                                 const PlacementId* pPlacementId, IUseAudioKeeper* pAudioKeeper,
                                 const bool* pIsReverseZoomInput)
    : mPlayerWatcher(pPlayerWatcher), mAudioKeeper(pAudioKeeper),
      mIsReverseZoomInput(pIsReverseZoomInput) {
    mNerveKeeper = new NerveKeeper(this, &NrvCameraPoserZoomWait, 0);
    mName = "Zoom";
    mPlacementId = new PlacementId(*pPlacementId);
    mFovyDegree = 25.0f;
    mStickPlayerFlag = 0xffff;
    _88 = true;
    mControlAngleParam = new ControlAngleParam();
    mZoomParams[0] = new CameraPoserZoomParam(2400.0f, 27.0f, {0.0f, 100.0f, 0.0f});
    mZoomParams[1] = new CameraPoserZoomParam(3300.0f, 31.0f, {0.0f, 100.0f, 0.0f});
    mZoomParams[2] = new CameraPoserZoomParam(5500.0f, 45.0f, {0.0f, 100.0f, 0.0f});
}

/**
 * Updates the look at position and the zoom state.
 */
void CameraPoserZoom::update() {
    updateLookAtPos();
    mNerveKeeper->update();
}

/**
 * Follows the owner player, leading the look at position slightly in the horizontal movement
 * direction.
 */
void CameraPoserZoom::updateLookAtPos() {
    sead::Vector3f playerPos = mPlayerWatcher->getPlayerPos(_9C);
    mLookAtPos = playerPos;

    sead::Vector3f moveDir = playerPos - mPrevPlayerPos;
    moveDir.y = 0.0f;
    f32 moveLength = moveDir.length();

    if (moveLength < 100.0f) {
        f32 offsetLength = moveLength * 25.0f;
        moveDir *= 25.0f;
        f32 prevOffsetLength = mLookAtOffsetLength;
        f32 diff = offsetLength - prevOffsetLength;

        if (sead::Mathf::abs(diff) > 30.0f) {
            offsetLength = prevOffsetLength + sgn(diff) * 30.0f;
        }

        normalizeOrZero(&moveDir);
        mLookAtPos += moveDir * offsetLength;
        mLookAtOffsetLength = offsetLength;
    }

    mPrevPlayerPos = playerPos;
}

/**
 * Writes the pose of the camera for the current zoom level.
 * @param pCamera The camera to write to.
 */
void CameraPoserZoom::makeLookAtCamera(sead::LookAtCamera* pCamera) const {
    const CameraPoserZoomParam& param = getZoomParam(mZoomLevel);
    sead::Vector3f dir = sead::Vector3f::ez;

    sead::Quatf quat;
    quat.setAxisAngle(-sead::Vector3f::ex, param.mAngleV);
    sead::Matrix34f mtx;
    mtx.fromQuat(quat);
    dir.setMul(mtx, dir);

    const f32& distance = mIsSnapshotMode ? mSnapshotDistance : param.mDistance;
    pCamera->setPos(mLookAtPos + dir * distance);
    pCamera->setAt(mLookAtPos + param.mOffsetLookAt);
    pCamera->setUp(mCameraUp);
    pCamera->normalizeUp();
}

/**
 * @param level Zoom level.
 * @return The parameters of a zoom level.
 */
const CameraPoserZoomParam& CameraPoserZoom::getZoomParam(s32 level) const {
    return *mZoomParams[level];
}

/**
 * Reads the parameters of the three zoom levels.
 * @param pIter The camera parameters.
 */
void CameraPoserZoom::loadParam(const ByamlIter* pIter) {
    CameraPoser::loadParam(pIter);
    pIter->tryGetBoolByKey(&_89, "IsValidUserCameraControl");

    tryLoadZoomParam(this, 0, pIter, "Near");
    tryLoadZoomParam(this, 1, pIter, "Normal");
    tryLoadZoomParam(this, 2, pIter, "Far");
}

/**
 * Starts the snapshot mode from the current zoom distance.
 * @param rate Interpolation rate used when a zoom interpolation is still in progress.
 */
void CameraPoserZoom::startSnapshotMode(f32 rate) {
    CameraPoser::startSnapshotMode(rate);
    mSnapshotZoomLevel = mZoomLevel;
    const CameraPoserZoomParam* param = mZoomParams[mZoomLevel];
    mControlAngleParam->mIsValid = true;
    mControlAngleParam->mAngleVLimitMax = 45.0f;
    mControlAngleParam->mAngleVLimitMin = 0.0f;

    if (isNerve(this, &NrvCameraPoserZoomInterpolate) && isLessStep(this, 15)) {
        mSnapshotStartDistance =
            lerpValue(rate, getZoomParam(mPrevZoomLevel).mDistance, param->mDistance);
    } else {
        mSnapshotStartDistance = param->mDistance;
    }

    mSnapshotDistance = mSnapshotStartDistance;
    mSnapshotTargetDistance = mSnapshotStartDistance;
    setNerve(this, &NrvCameraPoserZoomWait);
}

/**
 * Moves the snapshot zoom distance with the X and A buttons.
 * @return The field of view of the camera.
 */
f32 CameraPoserZoom::updateSnapshotFovy() {
    f32 prevDistance = mSnapshotDistance;

    if (isPadHoldX(getMainControllerPort())) {
        mSnapshotTargetDistance += -100.0f;
    } else if (isPadHoldA(getMainControllerPort())) {
        mSnapshotTargetDistance += 100.0f;
    }

    f32 nearDistance = mZoomParams[0]->mDistance;
    mSnapshotTargetDistance =
        sead::Mathf::clamp(mSnapshotTargetDistance, nearDistance, mZoomParams[2]->mDistance);
    mSnapshotDistance = lerpValueNew(mSnapshotDistance, mSnapshotTargetDistance, 0.6f);

    if (sead::Mathf::abs(mSnapshotDistance - prevDistance) > 30.0f) {
        f32 rate = (mSnapshotDistance - nearDistance) / (mZoomParams[2]->mDistance - nearDistance);
        tryHoldSeWithParam(mAudioKeeper, "PgZoom", 1.0f - rate, nullptr);
    }

    return mFovyDegree;
}

/**
 * @return The difference between the snapshot start distance and the current distance.
 */
f32 CameraPoserZoom::getSnapshotOffset() const {
    return mSnapshotStartDistance - mSnapshotDistance;
}

/**
 * Ends the snapshot mode and restores the zoom level from before it.
 */
void CameraPoserZoom::endSnapshotMode() {
    mControlAngleParam->mIsValid = false;
    CameraPoser::endSnapshotMode();
    mZoomLevel = mSnapshotZoomLevel;
}

/**
 * Disables zoom control by the user.
 */
void CameraPoserZoom::invalidControl() {
    mIsValidControl = false;

    if (!isNerve(this, &NrvCameraPoserZoomInterpolate)) {
        setNerve(this, &NrvCameraPoserZoomInvalid);
    }
}

/**
 * Enables zoom control by the user.
 */
void CameraPoserZoom::validControl() {
    mIsValidControl = true;

    if (!isNerve(this, &NrvCameraPoserZoomInterpolate)) {
        setNerve(this, &NrvCameraPoserZoomWait);
    }
}

/**
 * Returns to the normal zoom level.
 */
void CameraPoserZoom::resetZoomLevel() {
    if (mZoomLevel != 1) {
        mZoomLevel = 1;
        setNerve(this, &NrvCameraPoserZoomInterpolate);
    }
}

/**
 * Changes the zoom level with the right stick, or resets it with a stick press or L.
 */
void CameraPoserZoom::exeWait() {
    if (!_89 || mIsSnapshotMode) {
        return;
    }

    if (mZoomLevel != 1 && (isTriggerPressRightStickAny() || isTriggerLAny())) {
        mPrevZoomLevel = mZoomLevel;
        mZoomLevel = 1;
        mInterpolateFrame = 30;
        setNerve(this, &NrvCameraPoserZoomInterpolate);
        return;
    }

    s32 zoomDir;

    if (!tryGetZoomInput(&zoomDir, getMainControllerPort(), mIsReverseZoomInput) &&
        !(mStickPlayerFlag.isOnBit(1) && tryGetZoomInput(&zoomDir, 1, mIsReverseZoomInput)) &&
        !(mStickPlayerFlag.isOnBit(2) && tryGetZoomInput(&zoomDir, 2, mIsReverseZoomInput)) &&
        !(mStickPlayerFlag.isOnBit(3) && tryGetZoomInput(&zoomDir, 3, mIsReverseZoomInput))) {
        mIsStickReleased = true;
        return;
    }

    if (!mIsStickReleased) {
        return;
    }

    s32 zoomLevel = mZoomLevel + zoomDir;
    mIsStickReleased = false;

    if (static_cast<u32>(zoomLevel) > 2) {
        return;
    }

    mPrevZoomLevel = mZoomLevel;
    mZoomLevel = zoomLevel;
    mInterpolateFrame = 15;
    setNerve(this, &NrvCameraPoserZoomInterpolate);
}

/**
 * Plays the zoom sound and waits until the interpolation to the new zoom level is done.
 */
void CameraPoserZoom::exeInterpolate() {
    if (isFirstStep(this)) {
        if (mZoomLevel == 1) {
            startSe(mAudioKeeper, "UserMoveBack");
        } else {
            startSe(mAudioKeeper, "UserMove");
        }
    }

    if (isGreaterEqualStep(this, mInterpolateFrame)) {
        setNerve(this, mIsValidControl ? static_cast<const Nerve*>(&NrvCameraPoserZoomWait) :
                                         &NrvCameraPoserZoomInvalid);
    } else if (!mIsStickReleased && mIsValidControl) {
        s32 zoomDir;

        if (!tryGetZoomInput(&zoomDir, 1, mIsReverseZoomInput) &&
            !tryGetZoomInput(&zoomDir, 2, mIsReverseZoomInput) &&
            !tryGetZoomInput(&zoomDir, 3, mIsReverseZoomInput)) {
            mIsStickReleased = true;
        }
    }
}

/**
 * Does nothing while zoom control is disabled.
 */
void CameraPoserZoom::exeInvalid() {}

/**
 * @return The interpolation frames when switching to this camera.
 */
s32 CameraPoserZoom::getInterpoleApproachFrame() const {
    return 15;
}

/**
 * @return The interpolation frames when switching away from this camera.
 */
s32 CameraPoserZoom::getInterpoleGoAwayFrame() const {
    return 15;
}

/**
 * @return The parameters of the near zoom level.
 */
const CameraPoserZoomParam& CameraPoserZoom::getParamNear() const {
    return *mZoomParams[0];
}

/**
 * @return The parameters of the normal zoom level.
 */
const CameraPoserZoomParam& CameraPoserZoom::getParamNormal() const {
    return *mZoomParams[1];
}

/**
 * @return The parameters of the far zoom level.
 */
const CameraPoserZoomParam& CameraPoserZoom::getParamFar() const {
    return *mZoomParams[2];
}

/**
 * @return The default parameters of the near zoom level.
 */
const CameraPoserZoomParam& CameraPoserZoom::getDefaultParamNear() {
    return sDefaultParamNear;
}

/**
 * @return The default parameters of the normal zoom level.
 */
const CameraPoserZoomParam& CameraPoserZoom::getDefaultParamNormal() {
    return sDefaultParamNormal;
}

/**
 * @return The default parameters of the far zoom level.
 */
const CameraPoserZoomParam& CameraPoserZoom::getDefaultParamFar() {
    return sDefaultParamFar;
}

/**
 * @return The nerve keeper of the zoom state.
 */
NerveKeeper* CameraPoserZoom::getNerveKeeper() const {
    return mNerveKeeper;
}
}  // namespace al

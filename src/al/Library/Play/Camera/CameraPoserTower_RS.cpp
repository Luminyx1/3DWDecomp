#include "Library/Play/Camera/CameraPoserTower_RS.hpp"

#include <gfx/seadCamera.h>
#include <math/seadMathCalcCommon.h>

#include "Library/Camera/CameraPoserFunction.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Camera/CameraAngleCtrlInfo.hpp"
#include "Project/Camera/CameraOffsetPreset.hpp"

namespace {
using namespace al;

// The nerve keeper of a camera poser holds the poser itself, not its IUseNerve base.
#define POSER_NERVE_DECL(Class, Action)                                                            \
    class Class##Nrv##Action : public al::Nerve {                                                  \
    public:                                                                                        \
        void execute(al::NerveKeeper* pKeeper) const override {                                    \
            static_cast<Class*>(static_cast<void*>(pKeeper->getParent<al::IUseNerve>()))           \
                ->exe##Action();                                                                   \
        }                                                                                          \
    };

POSER_NERVE_DECL(CameraPoserTower_RS, Tower)
POSER_NERVE_DECL(CameraPoserTower_RS, Follow)
POSER_NERVE_DECL(CameraPoserTower_RS, TowerInput)

NERVES_MAKE_NOSTRUCT(CameraPoserTower_RS, Tower, Follow, TowerInput)

/**
 * Calculates the camera direction from its angles.
 * @param pDir Output direction.
 * @param angleH Horizontal angle in degrees.
 * @param angleV Vertical angle in degrees.
 */
inline void calcDirByAngle(sead::Vector3f* pDir, f32 angleH, f32 angleV) {
    pDir->set(sinf(sead::Mathf::deg2rad(angleH)) * cosf(sead::Mathf::deg2rad(angleV)),
              sinf(sead::Mathf::deg2rad(angleV)),
              cosf(sead::Mathf::deg2rad(angleH)) * cosf(sead::Mathf::deg2rad(angleV)));
    normalize(pDir);
}

}  // namespace

namespace al {

/**
 * Starts an interpolated rotation of the horizontal angle.
 * @param startAngleH Horizontal angle to rotate from.
 * @param endAngleH Horizontal angle to rotate to.
 * @param speed Rotation speed in degrees per frame.
 * @param minStep Minimum number of steps of the rotation.
 */
inline void CameraPoserTower_RS::startInterpRotate(f32 startAngleH, f32 endAngleH, f32 speed,
                                                   s32 minStep) {
    mResetStartAngleH = startAngleH;
    mResetEndAngleH = endAngleH;
    mResetStep = 0;
    mResetSpeed = speed;
    f32 diff = sead::Mathf::abs(diffNearAngleDegree(startAngleH, endAngleH));
    s32 step = minStep;

    if (!isNearZero(diff, 0.001f) && !isNearZero(speed, 0.001f)) {
        step = sead::Mathi::clampMin(static_cast<s32>(diff / speed), minStep);
    }

    mResetStepNum = step;
}

/**
 * @return Horizontal distance between the tower axis and the target.
 */
[[gnu::always_inline]] inline f32 CameraPoserTower_RS::calcAxisDistanceH() const {
    sead::Vector3f axisPos;
    calcAxisPos(&axisPos);
    sead::Vector3f targetTrans = {0.0f, 0.0f, 0.0f};
    alCameraPoserFunction::calcTargetTrans(&targetTrans, this);
    sead::Vector3f diff = axisPos - targetTrans;
    verticalizeVec(&diff, mUp, diff);
    return diff.length();
}

/**
 * @param distance Distance to check.
 * @return Whether the target is closer to the tower axis than the distance.
 */
[[gnu::always_inline]] inline bool CameraPoserTower_RS::isNearAxis(f32 distance) const {
    return calcAxisDistanceH() < distance;
}

/**
 * Creates a camera that circles around the axis of a tower.
 * @param pName Camera name.
 * @param pAxisPos Position of the tower axis, or nullptr to use the placement.
 */
CameraPoserTower_RS::CameraPoserTower_RS(const char* pName, const sead::Vector3f* pAxisPos)
    : CameraPoser_RS(pName), mAxisPosPtr(pAxisPos) {}

/**
 * Sets up the nerve and the camera helpers.
 */
void CameraPoserTower_RS::init() {
    initNerve(&NrvCameraPoserTower_RSTower, 0);
    initLocalInterpole();
    initLookAtInterpole(0.2f);
    alCameraPoserFunction::initCameraVerticalAbsorber(this);
    alCameraPoserFunction::initCameraAngleCtrl(this);
    alCameraPoserFunction::initCameraDefaultAngleRangeV(this, 0.0f, 85.0f);
    alCameraPoserFunction::initCameraArrowCollider(this);
    mFixLookAtDistanceInfo = new FixLookAtDistanceInfo();
    mOffsetPreset = new CameraOffsetPreset();
    alCameraPoserFunction::initSnapShotCameraCtrlZoomRollMove(this, false, false);
}

/**
 * Reads the tower axis from the linked placement.
 * @param rInfo Placement info of the camera.
 */
void CameraPoserTower_RS::initByPlacementObj(const PlacementInfo& rInfo) {
    PlacementInfo axisInfo;

    if (tryGetLinksInfo(&axisInfo, rInfo, "TowerCameraAxis")) {
        getTrans(&mAxisPos, axisInfo);
        mIsSetAxisPos = true;
    }

    tryInitAreaLimitter(rInfo);
}

/**
 * Loads the camera parameters.
 * @param rIter Camera parameter iterator.
 */
void CameraPoserTower_RS::loadParam(const ByamlIter& rIter) {
    mOffsetPreset->loadParam(rIter);
    tryGetByamlBool(&mFixLookAtDistanceInfo->isFix, rIter, "IsFixLookAtDistance");
    tryGetByamlF32(&mMarginAngleH, rIter, "MarginAngleH");
    tryGetByamlF32(&mDistance, rIter, "Distance");

    if (!tryGetByamlF32(&mDistanceNear, rIter, "DistanceNear")) {
        mDistanceNear = mDistance;
    }

    tryGetByamlF32(&mSwitchToFollowDistance, rIter, "SwitchToFollowDistance");
    tryGetByamlF32(&mInterpRotateSpeedByFrame, rIter, "InterpRotateSpeedByFrame");
    mIsValidVelocityOffset = tryGetByamlKeyBoolOrFalse(rIter, "IsValidVelocityOffset");

    if (mIsValidVelocityOffset) {
        tryGetByamlF32(&mVelocityOffsetMax, rIter, "VelocityOffsetMax");
    }

    mIsClampInMarginInputOffsetAngleH =
        tryGetByamlKeyBoolOrFalse(rIter, "IsClampInMarginInputOffsetAngleH");
    tryGetByamlV3f(&mLocalAxisPos, rIter, "AxisPos");
}

/**
 * Places the camera behind the target, following it if it is near the tower axis.
 * @param rInfo Start info.
 */
void CameraPoserTower_RS::start(const CameraStartInfo& rInfo) {
    mResetStep = -1;
    mResetStepNum = -1;
    mInputSpeedH = 0.0f;
    mVelocityOffset = 0.0f;
    mVelocityOffsetTarget = 0.0f;
    mCurrentMarginAngleH = mUserMarginAngleH >= 0.0f ? mUserMarginAngleH : mMarginAngleH;
    alCameraPoserFunction::calcTargetTrans(&mPrevTargetTrans, this);
    mAt.set(mPrevTargetTrans + calcOffset());
    sead::Vector3f preDirH = {0.0f, 0.0f, 0.0f};
    alCameraPoserFunction::calcPreCameraDirH(&preDirH, this);

    if (isNearAxis(mSwitchToFollowDistance)) {
        setNerve(this, &NrvCameraPoserTower_RSFollow);
        f32 preAngleH = alCameraPoserFunction::calcPreCameraAngleH(this);
        mTargetAngleH = preAngleH;
        mAngleH = preAngleH;
        alCameraPoserFunction::setCameraAngleV(this, alCameraPoserFunction::getCameraAngleV(this));
        sead::Vector3f dir;
        calcDirByAngle(&dir, mAngleH, alCameraPoserFunction::getCameraAngleV(this));
        mEye = mAt + dir * calcDistance();
        mTowerEye.set(mEye);
        return;
    }

    sead::Vector3f dirH = {0.0f, 0.0f, 0.0f};
    calcAxisPos(&mPrevAxisPos);

    if (calcDirH(&dirH, mPrevAxisPos, mAt)) {
        dirH.set(preDirH);
    }

    mAngleH = sead::Mathf::rad2deg(atan2f(preDirH.x, preDirH.z));
    mTargetAngleH = sead::Mathf::rad2deg(atan2f(dirH.x, dirH.z));
    f32 diff = diffNearAngleDegree(mTargetAngleH, mAngleH);

    if (!isInRange(diff, mCurrentMarginAngleH * -0.5f, mCurrentMarginAngleH * 0.5f)) {
        f32 halfMargin = mCurrentMarginAngleH * 0.5f;
        mAngleH = wrapValue(mAngleH + (diff > 0.0f ? halfMargin - diff : -diff - halfMargin),
                            360.0f);
    }

    mInterpAngleH = mAngleH;
    sead::Vector3f dir;
    calcDirByAngle(&dir, mAngleH, alCameraPoserFunction::getCameraAngleV(this));
    mEye = mAt + dir * calcDistance();
    mTowerEye.set(mEye);
    setNerve(this, &NrvCameraPoserTower_RSTowerInput);
}

/**
 * @return Offset of the look at position, interpolated while zooming in.
 */
sead::Vector3f CameraPoserTower_RS::calcOffset() {
    if (!mIsZoomIn) {
        return mOffsetPreset->getOffset();
    }

    sead::Vector3f offset = mOffsetPreset->getOffset();
    f32 rate = mZoomFrame == 0 ?
                   1.0f :
                   sead::Mathf::clamp(static_cast<f32>(mZoomStep) / mZoomFrame, 0.0f, 1.0f);
    offset.y = offset.y * (1.0f - rate) + rate * mZoomOffsetY;
    return offset;
}

/**
 * @return Camera distance depending on the vertical angle, interpolated while zooming in.
 */
f32 CameraPoserTower_RS::calcDistance() {
    if (isNear(mDistance, mDistanceNear, 0.001f)) {
        return mDistance;
    }

    f32 distance = mDistance;
    f32 distanceNear = mDistanceNear;

    if (mIsZoomIn) {
        f32 rate = mZoomFrame < 1 ? 1.0f :
                                    sead::Mathf::clamp(static_cast<f32>(mZoomStep) / mZoomFrame,
                                                       0.0f, 1.0f);
        distance = distance * (1.0f - rate) + rate * mZoomDistance;
        distanceNear = distanceNear * (1.0f - rate) + rate * mZoomDistanceNear;
        mZoomStep++;
    }

    const CameraAngleCtrlInfo* angleCtrlInfo = getAngleCtrlInfo();
    f32 minAngleV = angleCtrlInfo->getDefaultMinAngleV();
    f32 rangeV = angleCtrlInfo->getDefaultMaxAngleV() - minAngleV;
    f32 rateV = (angleCtrlInfo->getAngleV() - minAngleV) / rangeV;
    return distance * rateV + distanceNear * (1.0f - rateV);
}

/**
 * Sets the horizontal margin angle and the vertical angle.
 * @param marginAngleH Horizontal margin angle.
 * @param angleV Vertical angle.
 */
void CameraPoserTower_RS::setParams(f32 marginAngleH, f32 angleV) {
    mMarginAngleH = marginAngleH;
    getAngleCtrlInfo()->setAngleV(angleV);
}

/**
 * Updates the look at position and the horizontal angle input before running the nerve.
 */
void CameraPoserTower_RS::movement() {
    sead::Vector3f targetTrans = {0.0f, 0.0f, 0.0f};
    alCameraPoserFunction::calcTargetTrans(&targetTrans, this);
    targetTrans.y = lerpValue(0.3f, mPrevTargetTrans.y, targetTrans.y);
    mAt = targetTrans + calcOffset();
    sead::Vector3f axisPos = {0.0f, 0.0f, 0.0f};
    sead::Vector3f dirH = {0.0f, 0.0f, 0.0f};
    calcAxisPos(&axisPos);

    if (!calcDirH(&dirH, axisPos, mAt)) {
        mTargetAngleH = sead::Mathf::rad2deg(atan2f(dirH.x, dirH.z));
    }

    if (mFixLookAtDistanceInfo->isFix && mSwitchToFollowDistance < calcAxisDistanceH()) {
        sead::Vector3f diff = mAt - axisPos;
        sead::Vector3f axisDiff = {0.0f, 0.0f, 0.0f};
        verticalizeVec(&axisDiff, mUp, diff);
        diff -= axisDiff;
        f32 length = axisDiff.length();

        if (length > 0.0f) {
            axisDiff *= (mSwitchToFollowDistance + 1.0f) / length;
        }

        mAt = axisPos + axisDiff + diff;
    }

    if (!isResetting()) {
        sead::Vector2f stick = {0.0f, 0.0f};
        alCameraPoserFunction::calcCameraRolledRotateStick(&stick, this);
        f32 stickH = stick.x;
        f32 speed;

        if (isNearZero(stickH, 0.3f)) {
            speed = 0.0f;
        } else {
            f32 rateV = normalize(alCameraPoserFunction::getCameraAngleV(this), 60.0f, 75.0f);
            f32 maxSpeed = lerpValue(rateV, 1.0f, 0.75f) * -1.8f;
            speed = normalizeAbs(stickH, 0.3f, 1.0f) * maxSpeed;
        }

        f32 inputSpeedH = lerpValue(0.2f, mInputSpeedH, speed);
        mInputSpeedH = lerpValue(0.5f, mInputSpeedH, inputSpeedH);
    }

    f32 marginAngleH = mUserMarginAngleH >= 0.0f ? mUserMarginAngleH : mMarginAngleH;
    f32 currentMarginAngleH = lerpValue(0.2f, mCurrentMarginAngleH, marginAngleH);
    mCurrentMarginAngleH = lerpValue(0.1f, mCurrentMarginAngleH, currentMarginAngleH);
    CameraPoser_RS::movement();
    mPrevTargetTrans.set(targetTrans);
    mPrevAxisPos.set(axisPos);
}

/**
 * Places the camera around the look at position by the interpolated horizontal angle.
 */
void CameraPoserTower_RS::update() {
    if (alCameraPoserFunction::isTriggerCameraResetRotate(this) && mResetSpeed - 0.1f < 4.0f) {
        resetInputRotate(4.0f, 15);
    }

    sead::Vector3f dir;
    calcDirByAngle(&dir, mAngleH, alCameraPoserFunction::getCameraAngleV(this));
    mTowerEye = mAt + dir * calcDistance();

    if (isResetting()) {
        f32 prevRate = easeOut(easeInOut(normalize(
            static_cast<f32>(sead::Mathi::max(mResetStep - 1, 0)), 0.0f,
            static_cast<f32>(mResetStepNum))));
        f32 rate = easeOut(easeInOut(normalize(static_cast<f32>(mResetStep), 0.0f,
                                               static_cast<f32>(mResetStepNum))));
        mInterpAngleH = lerpDegree(mInterpAngleH, mAngleH,
                                   normalize(rate - prevRate, 0.0f, 1.0f - prevRate));
        mResetStep++;

        if (!isResetting()) {
            mResetStep = -1;
            mResetStepNum = -1;
        }
    } else {
        mInterpAngleH = mAngleH;
    }

    calcDirByAngle(&dir, mInterpAngleH, alCameraPoserFunction::getCameraAngleV(this));
    mEye = mAt + dir * calcDistance();
}

/**
 * Rotates the camera back behind the target.
 * @param speed Rotation speed in degrees per frame.
 * @param minStep Minimum number of steps of the rotation.
 */
void CameraPoserTower_RS::resetInputRotate(f32 speed, s32 minStep) {
    startInterpRotate(mInterpAngleH, mTargetAngleH, speed, minStep);
    mInputSpeedH = 0.0f;
    mTargetAngleH = mResetEndAngleH;
    mAngleH = mResetEndAngleH;

    if (alCameraPoserFunction::isSnapShotMode(this)) {
        alCameraPoserFunction::startResetSnapShotCameraCtrl(this, mResetStepNum);
    }
}

/**
 * Shifts the camera sideways by the velocity offset.
 * @param pCamera Camera to modify.
 */
void CameraPoserTower_RS::makeLookAtCamera(sead::LookAtCamera* pCamera) const {
    if (!mIsValidVelocityOffset) {
        return;
    }

    sead::Vector3f sideDir = {0.0f, 0.0f, 0.0f};
    alCameraPoserFunction::calcSideDir(&sideDir, this);
    sead::Vector3f offset = mVelocityOffset * sideDir;
    pCamera->addPos(offset);
    pCamera->addAt(offset);
}

/**
 * Turns the camera to keep the target inside the horizontal margin.
 */
void CameraPoserTower_RS::exeTower() {
    if (isFirstStep(this)) {
        mFollowSpeedH = 0.0f;
    }

    sead::Vector3f dirH = {0.0f, 0.0f, 0.0f};

    if (calcDirH(&dirH, mAt, mTowerEye)) {
        dirH.set(sinf(sead::Mathf::deg2rad(mAngleH)), 0.0f,
                 cosf(sead::Mathf::deg2rad(mAngleH)));
    }

    f32 angleH = mAngleH;
    f32 cameraAngleH = sead::Mathf::rad2deg(atan2f(dirH.x, dirH.z));
    sead::Vector3f targetTrans = {0.0f, 0.0f, 0.0f};
    alCameraPoserFunction::calcTargetTrans(&targetTrans, this);
    f32 halfMargin = mCurrentMarginAngleH * 0.5f;

    if (isNearZero(halfMargin, 0.001f)) {
        mAngleH = mTargetAngleH;
    } else {
        f32 diff = diffNearAngleDegree(mTargetAngleH, cameraAngleH);

        if (isInRange(diff, -halfMargin, halfMargin)) {
            mFollowSpeedH = mFollowSpeedH * 0.8f;
        } else {
            f32 overAngle = diff + (diff > 0.0f ? -halfMargin : halfMargin);
            f32 rate;

            if (isNearZero(halfMargin, 0.001f)) {
                rate = 1.0f;
            } else {
                rate = lerpValue(normalize(sead::Mathf::abs(overAngle), 0.0f, halfMargin), 0.05f,
                                 0.7f);
            }

            sead::Vector3f axisPos;
            calcAxisPos(&axisPos);
            f32 targetMove =
                sead::Mathf::sqrt(sead::Mathf::square(mPrevTargetTrans.x - targetTrans.x) +
                                  sead::Mathf::square(mPrevTargetTrans.z - targetTrans.z));
            f32 axisMove = sead::Mathf::sqrt(sead::Mathf::square(mPrevAxisPos.x - axisPos.x) +
                                             sead::Mathf::square(mPrevAxisPos.z - axisPos.z));
            f32 moveRate = normalize(sead::Mathf::max(targetMove, axisMove), 0.0f, 5.0f);
            mFollowSpeedH = moveRate * lerpValue(0.05f, mFollowSpeedH, overAngle * rate);
        }

        mAngleH = lerpDegree(angleH, wrapValue(cameraAngleH - mFollowSpeedH, 360.0f), 0.3f);
    }

    if (isNearAxis(mSwitchToFollowDistance)) {
        setNerve(this, &NrvCameraPoserTower_RSFollow);
        return;
    }

    if (mIsValidVelocityOffset) {
        sead::Vector3f lookDir = mEye - mAt;
        verticalizeVec(&lookDir, mUp, lookDir);

        if (tryNormalizeOrZero(&lookDir) && !isParallelDirection(lookDir, mUp, 0.01f)) {
            sead::Vector3f sideDir;
            sideDir.setCross(lookDir, mUp);
            normalize(&sideDir);
            sead::Vector3f move = targetTrans - mPrevTargetTrans;
            parallelizeVec(&move, sideDir, move);
            f32 dot = move.dot(sideDir);
            f32 moveRate = normalize(move.length(), 1.0f, 7.5f) * (dot > 0.0f ? 1.0f : -1.0f);
            f32 offset = sead::Mathf::clamp(mVelocityOffsetTarget + moveRate * 3.0f,
                                            -mVelocityOffsetMax, mVelocityOffsetMax);
            mVelocityOffsetTarget = lerpValue(0.7f, mVelocityOffsetTarget, offset);
            mVelocityOffset = lerpValue(0.2f, mVelocityOffset, mVelocityOffsetTarget);
        }
    }

    if (!isNearZero(mInputSpeedH, 0.001f)) {
        setNerve(this, &NrvCameraPoserTower_RSTowerInput);
    }
}

/**
 * Rotates the camera by the horizontal angle input.
 */
void CameraPoserTower_RS::exeTowerInput() {
    if (isFirstStep(this)) {
        mInputOffsetAngleH = diffNearAngleDegree(mTargetAngleH, mInterpAngleH);
        mResetStep = -1;
        mResetStepNum = -1;
    } else if (isResetting()) {
        mInputSpeedH = 0.0f;
        setNerve(this, &NrvCameraPoserTower_RSTower);
        return;
    }

    if (mIsClampInMarginInputOffsetAngleH) {
        if (!isNearZero(mInputSpeedH, 0.001f)) {
            f32 offsetAngle = sead::Mathf::clamp(mInputOffsetAngleH + mInputSpeedH * 1.8f,
                                                 mCurrentMarginAngleH * -0.5f,
                                                 mCurrentMarginAngleH * 0.5f);
            offsetAngle = lerpValue(0.7f, mInputOffsetAngleH, offsetAngle);
            mInputOffsetAngleH = lerpValue(0.5f, mInputOffsetAngleH, offsetAngle);
        }
    } else {
        mInputOffsetAngleH = mInputSpeedH + mInputOffsetAngleH;
    }

    mAngleH = wrapValue(mTargetAngleH + mInputOffsetAngleH, 360.0f);

    if (isNearAxis(mSwitchToFollowDistance)) {
        setNerve(this, &NrvCameraPoserTower_RSFollow);
    }
}

/**
 * Follows the target while it is near the tower axis.
 */
void CameraPoserTower_RS::exeFollow() {
    if (mIsValidVelocityOffset) {
        mVelocityOffsetTarget = lerpValue(0.7f, mVelocityOffsetTarget, 0.0f);
        mVelocityOffset = lerpValue(0.2f, mVelocityOffset, mVelocityOffsetTarget);
    }

    if (isResetting()) {
        return;
    }

    sead::Vector3f dirH = {0.0f, 0.0f, 0.0f};

    if (calcDirH(&dirH, mAt, mTowerEye)) {
        dirH.set(sinf(sead::Mathf::deg2rad(mAngleH)), 0.0f,
                 cosf(sead::Mathf::deg2rad(mAngleH)));
    }

    mAngleH = sead::Mathf::rad2deg(atan2f(dirH.x, dirH.z));

    if (!isResetting()) {
        mAngleH = wrapValue(mAngleH + mInputSpeedH, 360.0f);
    }

    if (!(mSwitchToFollowDistance + 150.0f < calcAxisDistanceH())) {
        return;
    }

    if (isInRange(diffNearAngleDegree(mTargetAngleH, mInterpAngleH) * 2.0f,
                  -mCurrentMarginAngleH, mCurrentMarginAngleH)) {
        setNerve(this, &NrvCameraPoserTower_RSTower);
        return;
    }

    f32 minAngleH = wrapValue(mTargetAngleH - mCurrentMarginAngleH * 0.5f, 360.0f);
    f32 maxAngleH = wrapValue(mTargetAngleH + mCurrentMarginAngleH * 0.5f, 360.0f);
    f32 diffMin = diffNearAngleDegree(mInterpAngleH, minAngleH);
    f32 diffMax = diffNearAngleDegree(mInterpAngleH, maxAngleH);
    bool isNearMin = sead::Mathf::abs(diffMin) < sead::Mathf::abs(diffMax);
    f32 diff = isNearMin ? sead::Mathf::abs(diffMin) : sead::Mathf::abs(diffMax);

    if (!isResetting() && !isNearZero(diff, 0.001f)) {
        startInterpRotate(
            mInterpAngleH,
            wrapValue(mTargetAngleH + mCurrentMarginAngleH * (isNearMin ? -0.5f : 0.5f), 360.0f),
            mInterpRotateSpeedByFrame, 60);
        mAngleH = mResetEndAngleH;
        sead::Vector3f dir;
        calcDirByAngle(&dir, mAngleH, alCameraPoserFunction::getCameraAngleV(this));
        mTowerEye = mAt + dir * calcDistance();
    }

    mInputSpeedH = 0.0f;
    setNerve(this, &NrvCameraPoserTower_RSTower);
}

/**
 * Sets whether the look at position keeps its distance to the tower axis.
 * @param isFix Whether the distance is fixed.
 */
void CameraPoserTower_RS::setFixToSwitchFollowDistance(bool isFix) {
    mFixLookAtDistanceInfo->setFix(isFix);
}

/**
 * Starts zooming in.
 * @param distance Camera distance at the highest vertical angle.
 * @param distanceNear Camera distance at the lowest vertical angle.
 * @param offsetY Height offset of the look at position.
 * @param frame Number of frames of the zoom.
 * @param isUnused Unused.
 */
void CameraPoserTower_RS::setZoomIn(f32 distance, f32 distanceNear, f32 offsetY, s32 frame,
                                    bool isUnused) {
    mZoomDistance = distance;
    mZoomDistanceNear = distanceNear;
    mZoomOffsetY = offsetY;
    mZoomFrame = frame;
    mZoomStep = 0;
    mIsZoomIn = true;
}

}  // namespace al

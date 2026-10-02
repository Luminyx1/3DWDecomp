#include "Library/Play/Camera/CameraPoserSubjective_RS.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/Camera/CameraPoserFunction.hpp"
#include "Library/Controller/PadRumbleFunction.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Math/IntervalTrigger.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Se/Function/SeFunction.hpp"
#include "Library/Yaml/ByamlUtil.hpp"

namespace {
using namespace al;

// The nerve keeper of a camera poser holds the poser itself, not its IUseNerve base.
#define POSER_NERVE_DECL(Class, Action)                                                            \
    class Class##Nrv##Action : public al::Nerve {                                                  \
    public:                                                                                        \
        void execute(al::NerveKeeper* pKeeper) const override {                                    \
            static_cast<Class*>(static_cast<void*>(pKeeper->getParent<al::IUseNerve>()))           \
                ->exe##Action();                                                           \
        }                                                                                          \
    };

POSER_NERVE_DECL(CameraPoserSubjective_RS, Wait)
POSER_NERVE_DECL(CameraPoserSubjective_RS, Reset)

NERVES_MAKE_NOSTRUCT(CameraPoserSubjective_RS, Wait, Reset)

}  // namespace

namespace al {

/**
 * Creates a first person camera.
 * @param pName Camera name.
 */
CameraPoserSubjective_RS::CameraPoserSubjective_RS(const char* pName) : CameraPoser_RS(pName) {}

/**
 * Initializes the nerve, the gyro and snapshot controllers and the move sound trigger.
 */
void CameraPoserSubjective_RS::init() {
    initNerve(&NrvCameraPoserSubjective_RSWait, 0);
    mFovyDegree = 45.0f;
    setInterpoleStep(10);
    alCameraPoserFunction::initGyroCameraCtrl(this);
    alCameraPoserFunction::initSnapShotCameraCtrl(this);
    mMoveSeTrigger = new IntervalTrigger(15.0f);
}

/**
 * Loads the vertical angle limits, the eye height and the start angles.
 * @param rIter Camera parameter iterator.
 */
void CameraPoserSubjective_RS::loadParam(const ByamlIter& rIter) {
    tryGetByamlF32(&mMinAngleV, rIter, "MinAngleV");
    tryGetByamlF32(&mMaxAngleV, rIter, "MaxAngleV");
    tryGetByamlF32(&mCameraOffsetUp, rIter, "CameraOffsetUp");
    tryGetByamlF32(&mStartAngleV, rIter, "StartAngleV");
    tryGetByamlBool(&mIsSetStartAngleH, rIter, "IsSetStartAngleH");
    tryGetByamlF32(&mStartAngleH, rIter, "StartAngleH");
}

/**
 * Takes over the angles of the previous camera, or uses the start angles.
 * @param rInfo Start info.
 */
void CameraPoserSubjective_RS::start(const CameraStartInfo& rInfo) {
    mFovyDegree = 45.0f;

    if (alCameraPoserFunction::isPrePriorityDemoAll(rInfo)) {
        mAngleV = -alCameraPoserFunction::calcPreCameraAngleV(this);
    } else {
        mAngleV = mStartAngleV;
    }

    mTargetAngleV = mAngleV;

    if (mIsSetStartAngleH && !alCameraPoserFunction::isPrePriorityDemoAll(rInfo)) {
        mAngleH = mStartAngleH;
    } else {
        sead::Vector3f lookDir = {0.0f, 0.0f, 0.0f};
        alCameraPoserFunction::calcPreLookDirH(&lookDir, this);
        mAngleH = sead::Mathf::rad2deg(atan2f(lookDir.x, lookDir.z));
    }

    mInputAngleH = 0.0f;
    _164 = 0.0f;
    mGyroAngleV = 0.0f;
    _160 = 0.0f;
    mInputSpeedH = 0.0f;
    mGyroAngleH = 0.0f;
    alCameraPoserFunction::resetGyro(this);
    alCameraPoserFunction::reduceGyroSencitivity(this);
    setNerve(this, &NrvCameraPoserSubjective_RSWait);
    update();
}

/**
 * Clears the zoom state before running the nerve.
 */
void CameraPoserSubjective_RS::movement() {
    mIsZooming = false;
    CameraPoser_RS::movement();
}

/**
 * Places the camera at the eye of the target and plays the move sound and rumble.
 */
void CameraPoserSubjective_RS::update() {
    sead::Vector3f dir = sead::Vector3f::ez;
    f32 angleH = wrapValue(mInputAngleH + mGyroAngleH + mAngleH, 360.0f);
    f32 angleV = sead::Mathf::clamp(mAngleV + mGyroAngleV, mMinAngleV, mMaxAngleV);
    rotateVectorDegree(&dir, dir, mUp, angleH);
    {
        sead::Vector3f side;
        side.setCross(dir, mUp);
        rotateVectorDegree(&dir, dir, side, angleV);
    }

    alCameraPoserFunction::setCameraPosToTargetAddOffset(
        this, dir * getCameraOffsetFront() + mCameraOffsetUp * mUp);
    mAt = mEye + dir * 100.0f;

    if (mSeActor != nullptr && !mIsSnapShotMode) {
        f32 moveAngle = sead::Mathf::abs(angleV - mPrevAngleV) +
                        sead::Mathf::abs(angleH - mPrevAngleH);
        mMoveSeTrigger->update(moveAngle);

        if (mMoveSeTrigger->isTriggered()) {
            f32 rate = moveAngle * 0.06f;
            f32 volume = sead::Mathf::clamp(rate + 0.1f, 0.0f, 1.0f);
            f32 pitch = sead::Mathf::clamp(rate + 0.25f, 0.0f, 1.0f);
            alPadRumbleFunction::startPadRumbleNo3DWithParam(mSeActor, "パルス（中）", volume,
                                                             pitch, 1.0f, 1.0f);
            startSeWithParam(mSeActor, "PgCameraMoveTrig", moveAngle, nullptr);
        }

        mPrevAngleV = angleV;
        mPrevAngleH = angleH;
    }
}

/**
 * Stops the gyro while in snapshot mode.
 */
void CameraPoserSubjective_RS::startSnapShotMode() {
    alCameraPoserFunction::stopUpdateGyro(this);
    mIsSnapShotMode = true;
}

/**
 * Restarts the gyro after snapshot mode.
 */
void CameraPoserSubjective_RS::endSnapShotMode() {
    alCameraPoserFunction::restartUpdateGyro(this);
    mIsSnapShotMode = false;
}

/**
 * Updates the zoom and rotates the camera by stick input.
 */
void CameraPoserSubjective_RS::exeWait() {
    f32 fovy = mFovyDegree;

    if (!alCameraPoserFunction::isSnapShotMode(this)) {
        if (mIsRequestZoomIn) {
            fovy = lerpValue(0.05f, fovy, 10.0f);
            mIsZooming = true;
            mIsRequestZoomIn = false;
        } else {
            fovy = lerpValue(0.15f, fovy, 45.0f);
        }

        mFovyDegree = fovy;
    }

    f32 zoomRate = 1.0f - normalize(fovy, 10.0f, 45.0f);
    sead::Vector2f stick = {0.0f, 0.0f};
    alCameraPoserFunction::calcCameraRolledRotateStick(&stick, this);
    mInputSpeedH = (mInputSpeedH - stick.x * lerpValue(zoomRate, 0.2f, 0.035f)) * 0.85f;
    mInputAngleH = wrapValue(mInputAngleH + mInputSpeedH, 360.0f);
    f32 targetAngleV = mTargetAngleV;
    f32 nextAngleV = lerpValue(
        0.7f, targetAngleV, targetAngleV + stick.y * lerpValue(zoomRate, 1.8f, 0.3f));
    mTargetAngleV = sead::Mathf::clamp(nextAngleV, mMinAngleV, mMaxAngleV);
    mAngleV = lerpValue(0.1f, mAngleV, mTargetAngleV);

    if (alCameraPoserFunction::isTriggerCameraResetRotate(this)) {
        setNerve(this, &NrvCameraPoserSubjective_RSReset);
        return;
    }

    alCameraPoserFunction::setGyroSensitivity(this, lerpValue(zoomRate, 1.5f, 0.75f),
                                              lerpValue(zoomRate, 1.75f, 1.0f));
}

/**
 * Returns the camera to the start angles.
 */
void CameraPoserSubjective_RS::exeReset() {
    if (isFirstStep(this)) {
        mInputAngleH = wrapValue(mInputAngleH + mGyroAngleH, 360.0f);
        mResetStartAngleH = mInputAngleH;
        mResetStartAngleV = mAngleV + mGyroAngleV;
        mTargetAngleV = mStartAngleV;
        alCameraPoserFunction::resetGyro(this);
        mGyroAngleH = 0.0f;
        _160 = 0.0f;
        _164 = 0.0f;
    }

    f32 rate = calcNerveEaseOutRate(this, 15);

    if (mIsValidResetAngleH) {
        mInputAngleH = lerpDegree(mResetStartAngleH, 0.0f, rate);
    }

    mAngleV = lerpValue(rate, mResetStartAngleV, mStartAngleV);

    if (isGreaterEqualStep(this, 15)) {
        setNerve(this, &NrvCameraPoserSubjective_RSWait);
    }
}

/**
 * @return Distance of the camera in front of the target.
 */
f32 CameraPoserSubjective_RS::getCameraOffsetFront() {
    return 50.0f;
}

/**
 * @return Whether the camera is zooming in.
 */
bool CameraPoserSubjective_RS::isZooming() const {
    return mIsZooming;
}

/**
 * @return Always true, the camera can be rotated by the pad.
 */
bool CameraPoserSubjective_RS::isEnableRotateByPad() const {
    return true;
}

}  // namespace al

#include "Library/Play/Camera/CameraPoserQuickTurn.hpp"

#include <gfx/seadCamera.h>
#include <math/seadMathCalcCommon.h>

#include "Library/Camera/CameraPoserFunction.hpp"
#include "Library/Controller/InputFunction.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Yaml/ByamlUtil.hpp"

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

POSER_NERVE_DECL(CameraPoserQuickTurn, Follow)
POSER_NERVE_DECL(CameraPoserQuickTurn, Race)

NERVES_MAKE_NOSTRUCT(CameraPoserQuickTurn, Follow, Race)

/**
 * Converts a stick tilt into a rotation, ignoring small tilts.
 * @param stick Horizontal stick tilt.
 * @param speed Rotation speed in degrees per frame at full tilt.
 * @return Rotation in degrees.
 */
inline f32 calcStickRotateDegree(f32 stick, f32 speed) {
    return sead::Mathf::abs(stick) < 0.3f ? 0.0f : -(speed * stick);
}

}  // namespace

namespace al {

/**
 * Constructs a camera that can be turned around the target with the stick.
 * @param pName Poser name.
 */
CameraPoserQuickTurn::CameraPoserQuickTurn(const char* pName) : CameraPoser_RS(pName) {}

/**
 * Remembers the default distance and starts following.
 */
void CameraPoserQuickTurn::init() {
    mDefaultDistance = mDistance;
    initNerve(&NrvCameraPoserQuickTurnFollow, 0);
}

/**
 * Loads the offset, distance, angle and reset flag.
 * @param rIter Camera parameter iterator.
 */
void CameraPoserQuickTurn::loadParam(const ByamlIter& rIter) {
    tryGetByamlF32(&mOffsetY, rIter, "OffsetY");
    tryGetByamlF32(&mDistance, rIter, "Distance");
    tryGetByamlF32(&mAngleDegreeV, rIter, "Angle");
    tryGetByamlBool(&mIsResetAngleIfSwitchTarget, rIter, "IsResetAngleIfSwitchTarget");
}

/**
 * Tilts the camera down and turns it by the horizontal angle while following.
 * @param pCamera Camera to write to.
 */
void CameraPoserQuickTurn::makeLookAtCamera(sead::LookAtCamera* pCamera) const {
    if (isNerve(this, &NrvCameraPoserQuickTurnRace)) {
        return;
    }

    sead::Vector3f dir = mEye - mAt;
    sead::Vector3f front = dir;
    tryNormalizeOrDirZ(&front);
    {
        sead::Vector3f side;
        side.setCross(front, sead::Vector3f::ey);
        rotateVectorDegree(&dir, dir, side, mAngleDegreeV);
    }
    f32 distance = mDistance;
    f32 length = dir.length();

    if (length > 0.0f) {
        dir *= distance / length;
    }

    sead::Vector3f back = -dir;
    rotateVectorDegreeY(&back, mAngleDegreeH);
    pCamera->setPos(mAt + -back);
}

/**
 * Resets the horizontal angle and follows the target.
 */
void CameraPoserQuickTurn::setFollow() {
    mAngleDegreeH = 0.0f;
    setNerve(this, &NrvCameraPoserQuickTurnFollow);
}

/**
 * Follows the target at a fixed distance and turns the horizontal angle with the left stick.
 */
void CameraPoserQuickTurn::exeFollow() {
    if (alCameraPoserFunction::isChangeTarget(this) && mIsResetAngleIfSwitchTarget) {
        reset();
    }

    alCameraPoserFunction::calcTargetTrans(&mAt, this);
    mAt.y = mOffsetY + mAt.y;
    sead::Vector3f dir = mEye - mAt;
    dir.y = 0.0f;

    if (isNearZero(dir, 0.001f)) {
        dir.set(sead::Vector3f::ez);
    }

    f32 distance = mDistance;
    f32 length = dir.length();

    if (length > 0.0f) {
        dir *= distance / length;
    }

    f32 rotateSpeed = mIsRotateFast ? 3.8f : 2.6f;
    mAngleDegreeH = wrapAngle(
        mAngleDegreeH + calcStickRotateDegree(getLeftStick(getMainControllerPort()).x, rotateSpeed));
    mEye = mAt + dir;
}

/**
 * Turns towards the moving direction of the target and places the camera behind it.
 */
void CameraPoserQuickTurn::exeRace() {
    if (isFirstStep(this)) {
        calcTargetFrontLocal(&mFrontDir, true);
    }

    sead::Vector3f front = {0.0f, 0.0f, 0.0f};
    calcTargetFrontLocal(&front, false);
    f32 angle = calcAngleOnPlaneDegree(mFrontDir, front, sead::Vector3f::ey) * 0.1f;
    mRotateAngle = lerpValue(0.3f, mRotateAngle, angle);
    rotateVectorDegreeY(&mFrontDir, mRotateAngle);
    mUp.set(sead::Vector3f::ey);
    alCameraPoserFunction::calcTargetTrans(&mAt, this);
    mAt.y = mOffsetY + mAt.y;
    sead::Vector3f dir = -mFrontDir;
    sead::Vector3f side;
    side.setCross(mFrontDir, sead::Vector3f::ey);
    side = -side;
    rotateVectorDegree(&dir, dir, side, mAngleDegreeV);
    f32 distance = mDistance;
    f32 length = dir.length();

    if (length > 0.0f) {
        dir *= distance / length;
    }

    mEye.set(mAt + dir);
}

/**
 * Calculates the horizontal direction the camera should face.
 * @param pFront Output direction.
 * @param isUseTargetFrontIfStopped Whether to fall back to the target front when the target does
 * not move.
 */
void CameraPoserQuickTurn::calcTargetFrontLocal(sead::Vector3f* pFront,
                                                bool isUseTargetFrontIfStopped) const {
    if (mFrontDirPtr != nullptr) {
        pFront->set(*mFrontDirPtr);
        return;
    }

    if (mIsTurnToVelocity) {
        sead::Vector3f velocity = {0.0f, 0.0f, 0.0f};
        alCameraPoserFunction::calcTargetVelocity(&velocity, this);
        velocity.y = 0.0f;

        if (tryNormalizeOrZero(&velocity)) {
            pFront->set(velocity);
            return;
        }

        if (isUseTargetFrontIfStopped) {
            alCameraPoserFunction::calcTargetFront(pFront, this);
        }

        return;
    }

    alCameraPoserFunction::calcTargetFront(pFront, this);
}

/**
 * Places the camera behind the target.
 */
void CameraPoserQuickTurn::reset() {
    alCameraPoserFunction::calcTargetTrans(&mAt, this);
    mAt.y = mOffsetY + mAt.y;
    sead::Vector3f front = {0.0f, 0.0f, 0.0f};
    alCameraPoserFunction::calcTargetFront(&front, this);
    mEye.set(mAt - front * mDistance);
}

/**
 * Places the camera behind the target.
 * @param rInfo Camera start info.
 */
void CameraPoserQuickTurn::start(const CameraStartInfo& rInfo) {
    reset();
}

}  // namespace al

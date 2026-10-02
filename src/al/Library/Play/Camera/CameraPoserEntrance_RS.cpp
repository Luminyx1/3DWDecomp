#include "Library/Play/Camera/CameraPoserEntrance_RS.hpp"

#include <attributes.h>
#include <math/seadMathCalcCommon.h>

#include "Library/Camera/CameraPoserFunction.hpp"
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

POSER_NERVE_DECL(CameraPoserEntrance_RS, Wait)
POSER_NERVE_DECL(CameraPoserEntrance_RS, KeepByFlag)
POSER_NERVE_DECL(CameraPoserEntrance_RS, KeepInAir)

NERVES_MAKE_NOSTRUCT(CameraPoserEntrance_RS, Wait, KeepByFlag, KeepInAir)

/**
 * Calculates the look at position of the camera.
 * @param pPoser Camera poser.
 * @param pParam Entrance camera parameter.
 * @param rTargetTrans Position of the target.
 * @return Look at position.
 */
inline sead::Vector3f calcLookAtPos(const CameraPoserEntrance_RS* pPoser,
                                    const CameraPoserEntrance_RS::Param* pParam,
                                    const sead::Vector3f& rTargetTrans) {
    sead::Vector3f lookAtPos = pParam->isSetLookAt ? pParam->lookAtPos : rTargetTrans;

    if (!isNearZero(pParam->lookAtOffset, 0.001f)) {
        sead::Vector3f offset = {0.0f, 0.0f, 0.0f};
        alCameraPoserFunction::rotateVecZone(&offset, pParam->lookAtOffset, pPoser);
        lookAtPos += offset;
    }

    return lookAtPos;
}

/**
 * Places the camera around the look at position by the angles and distance of the parameter.
 * @param pPoser Camera poser.
 * @param pParam Entrance camera parameter.
 * @param rTargetTrans Position of the target.
 */
void updateCameraPose(CameraPoserEntrance_RS* pPoser, const CameraPoserEntrance_RS::Param* pParam,
                      const sead::Vector3f& rTargetTrans) {
    sead::Vector3f dir = {0.0f, 0.0f, 0.0f};
    sead::Vector3f lookAtPos = calcLookAtPos(pPoser, pParam, rTargetTrans);
    dir.set(sinf(sead::Mathf::deg2rad(pParam->angleH)) *
                cosf(sead::Mathf::deg2rad(pParam->angleV)),
            sinf(sead::Mathf::deg2rad(pParam->angleV)),
            cosf(sead::Mathf::deg2rad(pParam->angleH)) *
                cosf(sead::Mathf::deg2rad(pParam->angleV)));
    setLength(&dir, pParam->distance);
    pPoser->setAt(lookAtPos);
    pPoser->setEye(lookAtPos + dir);
    pPoser->getUpPtr()->set(sead::Vector3f::ey);
}

/**
 * Checks whether the target moves enough to end the camera.
 * @param isCheckVelocity Whether the movement of the target is checked.
 * @param pPoser Camera poser.
 * @param rPrevTargetTrans Previous position of the target.
 * @return Whether the target is moving.
 */
NOINLINE bool isTargetMoving(u8 isCheckVelocity,
                                              const CameraPoserEntrance_RS* pPoser,
                                              const sead::Vector3f& rPrevTargetTrans) {
    if (!isCheckVelocity) {
        return true;
    }

    if (alCameraPoserFunction::calcTargetSpeedH(pPoser) > 2.5f) {
        return true;
    }

    sead::Vector3f move = {0.0f, 0.0f, 0.0f};
    alCameraPoserFunction::calcTargetTrans(&move, pPoser);
    move -= rPrevTargetTrans;
    sead::Vector3f gravity = {0.0f, 0.0f, 0.0f};
    alCameraPoserFunction::calcTargetGravity(&gravity, pPoser);
    verticalizeVec(&move, gravity, move);

    if (move.length() > 2.5f) {
        return true;
    }

    return alCameraPoserFunction::calcTargetJumpSpeed(pPoser) > 1.0f;
}

}  // namespace

namespace al {

/**
 * Creates a camera that shows the target when it enters a stage.
 * @param pName Camera name.
 */
CameraPoserEntrance_RS::CameraPoserEntrance_RS(const char* pName) : CameraPoser_RS(pName) {
    mParam = new Param;
    initNerve(&NrvCameraPoserEntrance_RSWait, 0);
}

/**
 * Sets the distance, the angles and the look at offset.
 * @param distance Camera distance.
 * @param angleH Horizontal angle in degrees.
 * @param angleV Vertical angle in degrees.
 * @param rLookAtOffset Offset of the look at position.
 */
void CameraPoserEntrance_RS::initParam(f32 distance, f32 angleH, f32 angleV,
                                       const sead::Vector3f& rLookAtOffset) {
    Param* param = mParam;
    param->angleH = angleH;
    param->angleV = angleV;
    param->distance = distance;
    param->lookAtOffset.set(rLookAtOffset);
}

/**
 * Sets the distance, the angles from a direction and the look at offset.
 * @param distance Camera distance.
 * @param rDir Direction from the look at position to the camera.
 * @param rLookAtOffset Offset of the look at position.
 */
void CameraPoserEntrance_RS::initParam(f32 distance, const sead::Vector3f& rDir,
                                       const sead::Vector3f& rLookAtOffset) {
    Param* param = mParam;
    param->distance = distance;
    param->angleV = sead::Mathf::rad2deg(asinf(rDir.y));
    f32 x = rDir.x;
    f32 z = rDir.z;
    f32 invCos = 1.0f / cosf(sead::Mathf::deg2rad(param->angleV));
    param->angleH = sead::Mathf::rad2deg(atan2f(x * invCos, z * invCos));
    param->lookAtOffset.set(rLookAtOffset);
}

/**
 * Sets a fixed look at position.
 * @param rLookAtPos Look at position.
 */
void CameraPoserEntrance_RS::initLookAtPosDirect(const sead::Vector3f& rLookAtPos) {
    Param* param = mParam;
    param->isSetLookAt = true;
    param->lookAtPos.set(rLookAtPos);
}

/**
 * Loads the camera parameters.
 * @param rIter Camera parameter iterator.
 */
void CameraPoserEntrance_RS::loadParam(const ByamlIter& rIter) {
    Param* param = mParam;
    tryGetByamlF32(&param->angleH, rIter, "AngleH");
    tryGetByamlF32(&param->angleV, rIter, "AngleV");
    tryGetByamlF32(&param->distance, rIter, "Distance");
    tryGetByamlV3f(&param->lookAtOffset, rIter, "LookAtOffset");
    tryGetByamlS32(&param->forceEndStep, rIter, "ForceEndStep");

    if (tryGetByamlBool(&param->isKeepInAir, rIter, "IsKeepInAir") && param->isKeepInAir) {
        tryGetByamlS32(&param->keepInAirCancelStep, rIter, "KeepInAirCancelStep");
        tryGetByamlBool(&param->isEndIfOnGround, rIter, "IsEndIfOnGround");
        tryGetByamlS32(&param->endDelayStepIfOnGround, rIter, "EndDelayStepIfOnGround");
    }

    if (tryGetByamlBool(&param->isSetLookAt, rIter, "IsSetLookAt") && param->isSetLookAt) {
        tryGetByamlV3f(&param->lookAtPos, rIter, "LookAtPos");
    } else {
        tryGetByamlBool(&param->isFollowTarget, rIter, "IsFollowTarget");
    }

    tryGetByamlBool(&param->isDisableEndIfNoVelocity, rIter, "IsDisableEndIfNoVelocity");
}

/**
 * Places the camera around the start position and selects the first nerve.
 * @param rInfo Start info.
 */
void CameraPoserEntrance_RS::start(const CameraStartInfo& rInfo) {
    mStep = 0;
    mValidInputStep = 0;
    mIsEndNotified = false;

    if (mIsSetStartPos) {
        mStartTargetTrans = mStartPos;
    } else {
        alCameraPoserFunction::calcTargetTrans(&mStartTargetTrans, this);
    }

    mTargetTrans.set(mStartTargetTrans);
    updateCameraPose(this, mParam, mStartTargetTrans);

    if (alCameraPoserFunction::isInvalidEndEntranceCamera(this)) {
        return setNerve(this, &NrvCameraPoserEntrance_RSKeepByFlag);
    }

    if (mParam->isKeepInAir) {
        return setNerve(this, &NrvCameraPoserEntrance_RSKeepInAir);
    }

    setNerve(this, &NrvCameraPoserEntrance_RSWait);
}

/**
 * Runs the nerve and keeps track of the target position.
 */
void CameraPoserEntrance_RS::movement() {
    CameraPoser_RS::movement();
    alCameraPoserFunction::calcTargetTrans(&mTargetTrans, this);
}

/**
 * Counts the steps while the camera is allowed to end.
 */
void CameraPoserEntrance_RS::update() {
    if (alCameraPoserFunction::isInvalidEndEntranceCamera(this)) {
        return;
    }

    mStep++;

    if (!alCameraPoserFunction::isTargetInvalidMoveByInput(this)) {
        mValidInputStep++;
    }
}

/**
 * Ends the camera and notifies the end once.
 */
void CameraPoserEntrance_RS::end() {
    CameraPoser_RS::end();

    if (!mIsEndNotified && mEndNotifier != nullptr && mEndNotifier->isEnableNotify) {
        mEndNotifier->isEnd = true;
        mIsEndNotified = true;
    }
}

/**
 * Keeps the camera while ending is invalidated.
 */
void CameraPoserEntrance_RS::exeKeepByFlag() {
    const Param* param = mParam;

    if (param->isFollowTarget) {
        sead::Vector3f targetTrans = {0.0f, 0.0f, 0.0f};
        alCameraPoserFunction::calcTargetTrans(&targetTrans, this);
        updateCameraPose(this, param, targetTrans);
    }

    if (alCameraPoserFunction::isInvalidEndEntranceCamera(this)) {
        return;
    }

    if (!mParam->isKeepInAir || alCameraPoserFunction::isTargetCollideGround(this)) {
        setNerve(this, &NrvCameraPoserEntrance_RSWait);
    } else {
        setNerve(this, &NrvCameraPoserEntrance_RSKeepInAir);
    }
}

/**
 * Keeps the camera while the target is in the air.
 */
void CameraPoserEntrance_RS::exeKeepInAir() {
    const Param* param = mParam;

    if (param->isFollowTarget) {
        sead::Vector3f targetTrans = {0.0f, 0.0f, 0.0f};
        alCameraPoserFunction::calcTargetTrans(&targetTrans, this);
        updateCameraPose(this, param, targetTrans);
    }

    if (alCameraPoserFunction::isTargetCollideGround(this)) {
        if (!mParam->isEndIfOnGround || mParam->endDelayStepIfOnGround > 0) {
            setNerve(this, &NrvCameraPoserEntrance_RSWait);
            return;
        }

        end();
        return;
    }

    if (mParam->forceEndStep >= 1 && mParam->forceEndStep <= mStep) {
        end();
        return;
    }

    s32 cancelStep = mParam->keepInAirCancelStep < 0 ? 60 : mParam->keepInAirCancelStep;

    if (cancelStep > mValidInputStep) {
        return;
    }

    setNerve(this, &NrvCameraPoserEntrance_RSWait);
}

/**
 * Sets the position the camera starts from.
 * @param rPos Start position.
 */
void CameraPoserEntrance_RS::setStartPos(sead::Vector3f& rPos) {
    mStartPos.set(rPos);
    updateCameraPose(this, mParam, mStartPos);
    mIsSetStartPos = true;
}

/**
 * Waits until the player moves the camera or the target, then ends the camera.
 */
void CameraPoserEntrance_RS::exeWait() {
    const Param* param = mParam;

    if (param->isFollowTarget) {
        sead::Vector3f targetTrans = {0.0f, 0.0f, 0.0f};
        alCameraPoserFunction::calcTargetTrans(&targetTrans, this);
        updateCameraPose(this, param, targetTrans);
    }

    sead::Vector3f targetTrans = {0.0f, 0.0f, 0.0f};
    alCameraPoserFunction::calcTargetTrans(&targetTrans, this);

    if (isFirstStep(this)) {
        if (mIsSetStartPos) {
            mWaitStartTargetTrans.set(mStartPos);
            mIsSetStartPos = false;
        } else {
            mWaitStartTargetTrans.set(targetTrans);
        }
    }

    if (alCameraPoserFunction::isInvalidEndEntranceCamera(this)) {
        setNerve(this, &NrvCameraPoserEntrance_RSKeepByFlag);
        return;
    }

    if (!alCameraPoserFunction::isActiveInterpole(this)) {
        if (alCameraPoserFunction::calcCameraRotateStickPower(this) > 0.3f ||
            alCameraPoserFunction::isTriggerCameraResetRotate(this)) {
            end();
            return;
        }

        f32 dx = mWaitStartTargetTrans.x - targetTrans.x;
        f32 moveDistance;

        if (mIsCheckMoveDistanceH) {
            f32 dz = mWaitStartTargetTrans.z - targetTrans.z;
            moveDistance = sead::Mathf::sqrt(dx * dx + dz * dz);
        } else {
            f32 dy = mWaitStartTargetTrans.y - targetTrans.y;
            f32 dz = mWaitStartTargetTrans.z - targetTrans.z;
            moveDistance = sead::Mathf::sqrt(dx * dx + dy * dy + dz * dz);
        }

        if (moveDistance > 10.0f) {
            if (isTargetMoving(mParam->isDisableEndIfNoVelocity, this, mTargetTrans)) {
                end();
            }

            return;
        }
    }

    if (mParam->isEndIfOnGround && isGreaterEqualStep(this, mParam->endDelayStepIfOnGround)) {
        end();
        return;
    }

    if (mParam->forceEndStep >= 1 && mParam->forceEndStep <= mStep) {
        end();
    }
}

/**
 * @return Always true, the camera can be rotated by the pad.
 */
bool CameraPoserEntrance_RS::isEnableRotateByPad() const {
    return true;
}

}  // namespace al

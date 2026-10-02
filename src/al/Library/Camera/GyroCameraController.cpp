#include "Library/Camera/GyroCameraController.hpp"

#include <gfx/seadCamera.h>
#include <math/seadMathCalcCommon.h>
#include <nerd/nerdMath.h>

#include "Library/Controller/InputFunction.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Se/Function/SeFunction.hpp"
#include "Project/Camera/ControlAngleParam.hpp"
#include "Project/Camera/Param/CameraFunction.hpp"

namespace {

/**
 * Calculates the vertical angle of the camera position seen from its look-at position.
 * @param rCamera The camera.
 * @return The vertical angle in radians.
 */
inline f32 calcCameraAngleV(const sead::LookAtCamera& rCamera) {
    sead::Vector3f dir = rCamera.getPos() - rCamera.getAt();
    sead::Vector3f dirH(dir.x, 0.0f, dir.z);
    f32 lengthH = nerd::sqrt(dirH.x * dirH.x + dirH.y * dirH.y + dirH.z * dirH.z);
    return sead::Mathf::atan2(dir.y, lengthH);
}

}  // namespace

namespace al {

/**
 * Creates the controller.
 * @param pAudioKeeper The audio keeper used to play the reset sound.
 * @param pCollisionDirector The collision director used to check the camera position.
 * @param pAreaObjDirector The area object director used to check the camera position.
 * @param pIsReverseH Whether the horizontal stick input is reversed.
 * @param pIsReverseV Whether the vertical stick input is reversed.
 */
GyroCameraController::GyroCameraController(IUseAudioKeeper* pAudioKeeper,
                                           CollisionDirector* pCollisionDirector,
                                           AreaObjDirector* pAreaObjDirector,
                                           const bool* pIsReverseH, const bool* pIsReverseV)
    : mCollisionDirector(pCollisionDirector), mAreaObjDirector(pAreaObjDirector),
      mAudioKeeper(pAudioKeeper), mIsReverseH(pIsReverseH), mIsReverseV(pIsReverseV) {}

/**
 * Sets the angle limits used when no valid parameter is given.
 * @param pDefaultParam The default angle parameter.
 */
void GyroCameraController::init(ControlAngleParam* pDefaultParam) {
    mDefaultParam = pDefaultParam;
}

/**
 * Starts the controller with the camera in its original orientation.
 */
void GyroCameraController::start() {
    mOffsetAngleV = -0.0f;
    mOffsetAngleH = -0.0f;
    mParam.mAngleV = 0.0f;
    mParam.mAngleH = 0.0f;
    mTargetAngleV = 0.0f;
    mTargetAngleH = 0.0f;
    mPrevAngleV = 0.0f;
}

/**
 * Calculates the angles given by the gyro. The gyro is not used, so both are zero.
 * @param pAngleH Where to store the horizontal angle.
 * @param pAngleV Where to store the vertical angle.
 */
void GyroCameraController::calcGyroAngleValue(f32* pAngleH, f32* pAngleV) {
    *pAngleV = 0.0f;
    *pAngleH = 0.0f;
}

/**
 * Updates the rotation angles and moves the camera position accordingly.
 * @param pCamera The camera to rotate.
 * @param pParam The angle limits to use if valid.
 * @param isUseParam Whether to limit the angles by a parameter instead of fixing them.
 */
void GyroCameraController::update(sead::LookAtCamera* pCamera, ControlAngleParam* pParam,
                                  bool isUseParam) {
    // The gyro input of the main controller is no longer read.
    getMainControllerPort();
    f32 gyroAngleH = 0.0f;
    f32 gyroAngleV = 0.0f;
    calcGyroAngleValue(&gyroAngleH, &gyroAngleV);

    if (mGyroAngleH * gyroAngleH < -10000.0f) {
        mOffsetAngleH += -360.0f;
    }

    mGyroAngleH = 0.0f;

    if (mGyroAngleV * gyroAngleV < -10000.0f) {
        mOffsetAngleV += -360.0f;
    }

    mGyroAngleV = 0.0f;

    setParam(pCamera, pParam, isUseParam);

    if (mIsRecover) {
        updateRecover(*pCamera, 0.0f, 0.0f);
    } else {
        updateGyroParams(*pCamera, 0.0f, 0.0f, isUseParam);
    }

    sead::Vector3f cameraPos;
    CameraFunction::calcCameraPosByRotateAngleHV(&cameraPos, *pCamera, mParam.mAngleV,
                                                 mParam.mAngleH);
    pCamera->setPos(cameraPos);
}

/**
 * Copies the angle limits of a parameter.
 * @param rParam The parameter to copy the limits from.
 */
inline void GyroCameraController::setAngleLimit(const ControlAngleParam& rParam) {
    mParam.mAngleVLimitMin = rParam.mAngleVLimitMin;
    mParam.mAngleVLimitMax = rParam.mAngleVLimitMax;
    mParam.mAngleHLimitMin = rParam.mAngleHLimitMin;
    mParam.mAngleHLimitMax = rParam.mAngleHLimitMax;
}

/**
 * Sets the angle limits from a parameter, or fixes the angles to the current camera.
 * @param pCamera The camera.
 * @param pParam The angle limits to use if valid.
 * @param isUseParam Whether to limit the angles by a parameter instead of fixing them.
 */
void GyroCameraController::setParam(sead::LookAtCamera* pCamera, ControlAngleParam* pParam,
                                    bool isUseParam) {
    f32 angleV = calcCameraAngleV(*pCamera);

    if (!isUseParam) {
        f32 angleVDegree = sead::Mathf::rad2deg(angleV);
        mParam.mAngleVLimitMin = angleVDegree;
        mParam.mAngleVLimitMax = angleVDegree;
        mParam.mAngleHLimitMin = 0.0f;
        mParam.mAngleHLimitMax = 0.0f;
    } else if (pParam->mIsValid) {
        setAngleLimit(*pParam);
    } else {
        setAngleLimit(*mDefaultParam);
    }
}

/**
 * Moves the camera back to its original orientation after it ended up inside collision.
 * @param rCamera The camera.
 * @param baseAngleV The vertical angle the offset is relative to.
 * @param baseAngleH The horizontal angle the offset is relative to.
 */
void GyroCameraController::updateRecover(const sead::LookAtCamera& rCamera, f32 baseAngleV,
                                         f32 baseAngleH) {
    f32 rate = easeInOut(mRecoverFrame / 30.0f);
    mParam.mAngleV = lerpValue(rate, mRecoverStartAngleV, 0.0f);
    mParam.mAngleH = lerpValue(rate, mRecoverStartAngleH, 0.0f);

    if (mRecoverFrame > 10) {
        if (CameraFunction::isInCollisionCameraPos(this, this, rCamera, mParam.mAngleV,
                                                   mParam.mAngleH)) {
            if (mRecoverFrame >= 30) {
                return;
            }
        } else {
            mRecoverFrame = 0;
            mIsRecover = false;
            mOffsetAngleV = mParam.mAngleV - baseAngleV;
            mOffsetAngleH = mParam.mAngleH - baseAngleH;
            mTargetAngleV = mParam.mAngleV;
            mTargetAngleH = mParam.mAngleH;
        }
    }

    mRecoverFrame++;
}

/**
 * Updates the rotation angles from the stick input and keeps them inside the angle limits.
 * @param rCamera The camera.
 * @param baseAngleV The vertical angle the offset is relative to.
 * @param baseAngleH The horizontal angle the offset is relative to.
 * @param isUseParam Whether the angles are limited by a parameter.
 */
void GyroCameraController::updateGyroParams(const sead::LookAtCamera& rCamera, f32 baseAngleV,
                                            f32 baseAngleH, bool isUseParam) {
    if (isPadTriggerPressRightStick(getMainControllerPort())) {
        if (isUseParam) {
            startSe(mAudioKeeper, "ResetGyro");
        }

        resetGyroParams();
    }

    f32 cameraAngleV = sead::Mathf::rad2deg(calcCameraAngleV(rCamera));
    sead::Vector2f stick = sead::Vector2f::zero;
    if (calcInputStick(&stick, getMainControllerPort()) || calcInputStick(&stick, 1) ||
        calcInputStick(&stick, 2) || calcInputStick(&stick, 3) || calcInputStick(&stick, 4)) {
        if (*mIsReverseV) {
            stick.y = -stick.y;
        }

        if (*mIsReverseH) {
            stick.x = -stick.x;
        }

        f32 stickAbsX = sead::Mathf::abs(stick.x);

        if (stickAbsX > 0.3f || sead::Mathf::abs(stick.y) > 0.3f) {
            if (stickAbsX < sead::Mathf::abs(stick.y)) {
                mOffsetAngleV = sead::Mathf::clamp(
                    mOffsetAngleV, mParam.mAngleVLimitMin - cameraAngleV - baseAngleV,
                    mParam.mAngleVLimitMax - cameraAngleV - baseAngleV);

                if (isInRange(cameraAngleV + mOffsetAngleV + baseAngleV - stick.y * 4.5f,
                              mParam.mAngleVLimitMin, mParam.mAngleVLimitMax)) {
                    mOffsetAngleV -= stick.y * 4.5f;
                }
            } else {
                mOffsetAngleH =
                    sead::Mathf::clamp(mOffsetAngleH, mParam.mAngleHLimitMin - baseAngleH,
                                       mParam.mAngleHLimitMax - baseAngleH);

                if (isInRange(mOffsetAngleH + baseAngleH - stick.x * 4.5f, mParam.mAngleHLimitMin,
                              mParam.mAngleHLimitMax)) {
                    mOffsetAngleH -= stick.x * 4.5f;
                }
            }
        }
    }

    f32 targetAngleV = mOffsetAngleV + baseAngleV;

    if (cameraAngleV + targetAngleV < mParam.mAngleVLimitMin + 1.0f) {
        targetAngleV = mParam.mAngleVLimitMin - cameraAngleV;
        mOffsetAngleV = targetAngleV - baseAngleV;
    } else if (cameraAngleV + targetAngleV > mParam.mAngleVLimitMax - 1.0f) {
        targetAngleV = mParam.mAngleVLimitMax - cameraAngleV;
        mOffsetAngleV = targetAngleV - baseAngleV;
    }

    mTargetAngleV = lerpValue(0.075f, mTargetAngleV, targetAngleV);
    mParam.mAngleV = lerpValue(0.075f, mParam.mAngleV, mTargetAngleV);
    f32 deltaAngleV = mParam.mAngleV - mPrevAngleV;

    if (deltaAngleV > 0.0f && cameraAngleV + mParam.mAngleV > mParam.mAngleVLimitMax) {
        mParam.mAngleV = mParam.mAngleVLimitMax - cameraAngleV;
    } else if (deltaAngleV < 0.0f && cameraAngleV + mParam.mAngleV < mParam.mAngleVLimitMin) {
        mParam.mAngleV = mParam.mAngleVLimitMin - cameraAngleV;
    }

    if (cameraAngleV + mParam.mAngleV > 89.0f) {
        mParam.mAngleV = 89.0f - cameraAngleV;
    }

    mPrevAngleV = mParam.mAngleV;
    f32 targetAngleH = mOffsetAngleH + baseAngleH;

    if (targetAngleH < mParam.mAngleHLimitMin + 1.0f) {
        targetAngleH = mParam.mAngleHLimitMin;
        mOffsetAngleH = targetAngleH - baseAngleH;
    } else if (targetAngleH > mParam.mAngleHLimitMax - 1.0f) {
        targetAngleH = mParam.mAngleHLimitMax;
        mOffsetAngleH = targetAngleH - baseAngleH;
    }

    mTargetAngleH = lerpValue(0.1f, mTargetAngleH, targetAngleH);
    mParam.mAngleH = lerpValue(0.1f, mParam.mAngleH, mTargetAngleH);

    if (isUseParam && CameraFunction::isInCollisionCameraPos(this, this, rCamera, mParam.mAngleV,
                                                             mParam.mAngleH)) {
        mIsRecover = true;
        mRecoverStartAngleV = mParam.mAngleV;
        mRecoverStartAngleH = mParam.mAngleH;
    }
}

/**
 * Resets the rotation offsets and their targets.
 */
void GyroCameraController::resetGyroParams() {
    mOffsetAngleV = -0.0f;
    mOffsetAngleH = -0.0f;
    mTargetAngleV = 0.0f;
    mTargetAngleH = 0.0f;
}

/**
 * Reads the right stick of a controller, or the directional buttons of other controllers.
 * @param pOut Where to store the stick input.
 * @param port The controller port.
 * @return Whether the stick is tilted far enough.
 */
bool GyroCameraController::calcInputStick(sead::Vector2f* pOut, s32 port) {
    sead::Vector2f stick = sead::Vector2f::zero;

    if (isPadEnableRightStick(port)) {
        stick = getRightStick(port);
    } else if (getMainControllerPort() != port) {
        if (isPadHoldUp(port)) {
            stick.y = 1.0f;
        }

        if (isPadHoldDown(port)) {
            stick.y = -1.0f;
        }

        if (isPadHoldLeft(port)) {
            stick.x = -1.0f;
        }

        if (isPadHoldRight(port)) {
            stick.x = 1.0f;
        }
    }

    if (sead::Mathf::abs(stick.x) < 0.3f && sead::Mathf::abs(stick.y) < 0.3f) {
        *pOut = sead::Vector2f::zero;
        return false;
    }

    *pOut = stick;
    return true;
}

/**
 * Creates the parameters with their default angles and limits.
 */
GyroCameraControllerParam::GyroCameraControllerParam()
    : mAngleV(0.0f), mAngleH(0.0f), mAngleVLimitMin(10.0f), mAngleVLimitMax(45.0f),
      mAngleHLimitMin(-45.0f), mAngleHLimitMax(45.0f) {}

}  // namespace al

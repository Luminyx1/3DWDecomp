#include "Library/Controller/PadGyroAddon.hpp"

#include <controller/nin/seadNinJoyNpadDevice.h>
#include <controller/seadControllerMgr.h>

#include "Library/Controller/NpadController.hpp"

namespace al {
/**
 * Creates a gyro addon for a six-axis sensor of an Npad controller.
 * @param pController Npad controller
 * @param index six-axis sensor index
 */
PadGyroAddon::PadGyroAddon(sead::Controller* pController, s32 index)
    : sead::ControllerAddon(pController), mIndex(index) {
    mId = sead::ControllerDefine::cAddon_Gyro;
}

/**
 * Updates the gyro status.
 * @return always false
 */
bool PadGyroAddon::calc() {
    mIsStatusOk = tryUpdateGyroStatus();
    return false;
}

/**
 * Reads the pose, angular velocity and angle of the controller's six-axis sensor.
 * @return whether the sensor state could be read
 */
bool PadGyroAddon::tryUpdateGyroStatus() {
    sead::NinJoyNpadDevice* device =
        sead::ControllerMgr::instance()->getControlDeviceAs<sead::NinJoyNpadDevice*>();
    NpadController* npad = static_cast<NpadController*>(mController);

    if (!npad->isConnected()) {
        return false;
    }

    s64 index = mIndex;
    if (index >= npad->getSixAxisSensorNum()) {
        return false;
    }

    const nn::hid::SixAxisSensorState& state =
        device->getNpadState(npad->getNpadId()).mSixAxisSensorStates[index].state[0];

    s64 samplingNumber = state.mSamplingNumber;
    mSampleCount = sead::Mathi::clamp(static_cast<s32>(samplingNumber - mPrevSamplingNumber), 0, 16);
    mPrevSamplingNumber = samplingNumber;

    const f32(*mtx)[3] = state.mDirection.mMtx;
    mSDKSide.set(mtx[0][0], mtx[0][1], mtx[0][2]);
    mSDKUp.set(mtx[1][0], mtx[1][1], mtx[1][2]);
    mSDKFront.set(mtx[2][0], mtx[2][1], mtx[2][2]);

    mAngularVelocity.set(-state.mAngularVelocity[0], state.mAngularVelocity[2],
                         state.mAngularVelocity[1]);
    mAngle.set(-state.mAngle[0], state.mAngle[2], state.mAngle[1]);

    mSide.set(mtx[0][0], -mtx[0][2], -mtx[0][1]);
    mUp.set(-mtx[2][0], mtx[2][2], mtx[2][1]);
    mFront.set(-mtx[1][0], mtx[1][2], mtx[1][1]);

    if (device->getNpadJoyHoldType() == nn::hid::NpadJoyHoldType::Horizontal) {
        sead::NinJoyNpadDevice::Style style = npad->getStyle();
        if (style == sead::NinJoyNpadDevice::cStyle_JoyLeft ||
            style == sead::NinJoyNpadDevice::cStyle_JoyRight) {
            bool isRight = style == sead::NinJoyNpadDevice::cStyle_JoyRight;

            f32 velocityX = -mAngularVelocity.x;
            f32 angleX = -mAngle.x;
            if (isRight) {
                mAngularVelocity.x = -mAngularVelocity.z;
                mAngularVelocity.z = -velocityX;
                mAngle.x = -mAngle.z;
                mAngle.z = -angleX;
            } else {
                mAngularVelocity.x = mAngularVelocity.z;
                mAngularVelocity.z = velocityX;
                mAngle.x = mAngle.z;
                mAngle.z = angleX;
            }

            sead::Vector3f side = mSide;
            sead::Vector3f up = mUp;
            sead::Vector3f front = mFront;
            if (isRight) {
                mSide.set(front.z, -front.y, -front.x);
                mUp.set(-up.z, up.y, up.x);
                mFront.set(-side.z, side.y, side.x);
            } else {
                mSide.set(front.z, front.y, -front.x);
                mUp.set(up.z, up.y, -up.x);
                mFront.set(-side.z, -side.y, side.x);
            }
        }
    }

    return true;
}

/**
 * Returns the controller pose in the game's coordinate system.
 * @param pSide side vector, may be nullptr
 * @param pUp up vector, may be nullptr
 * @param pFront front vector, may be nullptr
 */
void PadGyroAddon::getPose(sead::Vector3f* pSide, sead::Vector3f* pUp,
                           sead::Vector3f* pFront) const {
    if (pSide) {
        pSide->set(mSide);
    }

    if (pUp) {
        pUp->set(mUp);
    }

    if (pFront) {
        pFront->set(mFront);
    }
}

/**
 * Returns the controller pose as reported by the SDK.
 * @param pSide side vector, may be nullptr
 * @param pUp up vector, may be nullptr
 * @param pFront front vector, may be nullptr
 */
void PadGyroAddon::getSDKPose(sead::Vector3f* pSide, sead::Vector3f* pUp,
                              sead::Vector3f* pFront) const {
    if (pSide) {
        pSide->set(mSDKSide);
    }

    if (pUp) {
        pUp->set(mSDKUp);
    }

    if (pFront) {
        pFront->set(mSDKFront);
    }
}
}  // namespace al

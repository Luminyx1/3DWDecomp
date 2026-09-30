#include "Project/Controller/JoyPadAccelerometerAddon.hpp"

#include <controller/nin/seadNinJoyNpadDevice.h>
#include <controller/seadControllerMgr.h>

#include "Library/Controller/NpadController.hpp"

namespace al {
/**
 * Creates an accelerometer addon for a six-axis sensor of an Npad controller.
 * @param pController Npad controller
 * @param index six-axis sensor index
 */
JoyPadAccelerometerAddon::JoyPadAccelerometerAddon(sead::Controller* pController, s32 index)
    : sead::AccelerometerAddon(pController), mIndex(index) {}

/**
 * Reads the acceleration of the controller's six-axis sensor.
 * @return always false
 */
bool JoyPadAccelerometerAddon::calc() {
    sead::NinJoyNpadDevice* device =
        sead::ControllerMgr::instance()->getControlDeviceAs<sead::NinJoyNpadDevice*>();
    NpadController* npad = static_cast<NpadController*>(mController);

    mIsEnable = false;
    mAcceleration = {0.0f, 0.0f, 0.0f};

    if (!npad->isConnected()) {
        return false;
    }

    s64 index = mIndex;
    if (index >= npad->getSixAxisSensorNum()) {
        return false;
    }

    if (npad->getAccelerometerWaitCount() >= 1) {
        mWaitCount = 5;
        return false;
    }

    if (mWaitCount - 1 >= 0) {
        mWaitCount--;
        return false;
    }

    const nn::hid::SixAxisSensorState& state =
        device->getNpadState(npad->getNpadId()).mSixAxisSensorStates[index].state[0];
    if (state.mDeltaTime == 0) {
        return false;
    }

    f32 accelX = state.mAcceleration[0];
    f32 accelZ = state.mAcceleration[1];
    mAcceleration.y = state.mAcceleration[2];
    mAcceleration.z = accelZ;
    mAcceleration.x = -accelX;

    if (device->getNpadJoyHoldType() == nn::hid::NpadJoyHoldType::Horizontal) {
        sead::NinJoyNpadDevice::Style style = npad->getStyle();
        if (style == sead::NinJoyNpadDevice::cStyle_JoyLeft ||
            style == sead::NinJoyNpadDevice::cStyle_JoyRight) {
            f32 temp = -mAcceleration.x;
            if (style == sead::NinJoyNpadDevice::cStyle_JoyRight) {
                mAcceleration.x = -mAcceleration.z;
                mAcceleration.z = -temp;
            } else {
                mAcceleration.x = mAcceleration.z;
                mAcceleration.z = temp;
            }
        }
    }

    mDeltaTime = state.mDeltaTime;
    mIsEnable = true;
    return false;
}
}  // namespace al

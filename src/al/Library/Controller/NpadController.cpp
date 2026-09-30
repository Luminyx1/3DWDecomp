#include "Library/Controller/NpadController.hpp"

#include <controller/seadControllerMgr.h>

namespace al {
namespace {
inline sead::NinJoyNpadDevice* getNpadDevice() {
    return sead::ControllerMgr::instance()->getControlDeviceAs<sead::NinJoyNpadDevice*>();
}
}  // namespace

/**
 * Creates an Npad controller.
 * @param pMgr controller manager
 */
NpadController::NpadController(sead::ControllerMgr* pMgr) : sead::Controller(pMgr) {
    mId = sead::ControllerDefine::cController_Npad;
    setLeftStickCrossThreshold(0.5f, 0.25f);
    setRightStickCrossThreshold(0.5f, 0.25f);
}

/**
 * Checks whether the controller is assigned to a connected Npad.
 * @return whether it is connected
 */
bool NpadController::isConnected() const {
    if (mNpadId == -1) {
        return false;
    }

    return mIsConnected;
}

/**
 * Checks whether the controller is assigned to an Npad.
 * @return whether the Npad id is valid
 */
bool NpadController::isValidNpadId() const {
    return mNpadId != -1;
}

/**
 * Returns the assigned Npad id.
 * @return Npad id, or -1
 */
s32 NpadController::getNpadId() const {
    return mNpadId;
}

/**
 * Lets the controller use any Npad.
 */
void NpadController::setAnyControllerMode() {
    setIndexControllerMode_(-1);
}

/**
 * Lets the controller use any Npad.
 * @param isFlag stored flag
 */
void NpadController::setAnyControllerMode(bool isFlag) {
    setIndexControllerMode_(-1);
    _190 = isFlag;
}

/**
 * Assigns the controller to an Npad index.
 * @param index Npad index
 */
void NpadController::setIndexControllerMode(s32 index) {
    setIndexControllerMode_(index);
}

/**
 * Assigns the controller to an Npad index.
 * @param index Npad index
 * @param isFlag stored flag
 */
void NpadController::setIndexControllerMode(s32 index, bool isFlag) {
    setIndexControllerMode_(index);
    _190 = isFlag;
}

/**
 * Stores the controller mode index.
 * @param index Npad index, or -1 for any
 */
void NpadController::setIndexControllerMode_(s32 index) {
    mControllerModeIndex = index;
}

/**
 * Returns a six-axis sensor handle of the assigned Npad.
 * @param index sensor index
 * @return the sensor handle
 */
const nn::hid::SixAxisSensorHandle& NpadController::getSixAxisSensorHandle(s32 index) const {
    sead::NinJoyNpadDevice* device = getNpadDevice();
    return device->getNpadState(mNpadId).mSixAxisSensorHandles[index];
}

/**
 * Returns a vibration device handle of the assigned Npad.
 * @param index device index
 * @return the vibration device handle
 */
const nn::hid::VibrationDeviceHandle& NpadController::getVibrationDeviceHandle(s32 index) const {
    sead::NinJoyNpadDevice* device = getNpadDevice();
    return device->getNpadState(mNpadId).mVibrationDeviceHandles[index];
}

/**
 * Checks whether a six-axis sensor is at rest.
 * @param index sensor index
 * @return whether the sensor is at rest, true without an Npad
 */
bool NpadController::isSixAxisSensorAtRest(s32 index) const {
    if (mNpadId == -1) {
        return true;
    }

    return nn::hid::IsSixAxisSensorAtRest(getSixAxisSensorHandle(index));
}

/**
 * Clears the input state.
 */
void NpadController::resetInput() {
    mPrevStyle = mStyle;
    mSixAxisSensorNum = 0;
    _18c = 0;
    mStyle = sead::NinJoyNpadDevice::cStyle_Invalid;
    mPadHold.makeAllZero();
    mLeftStick.set(0.0f, 0.0f);
    mRightStick.set(0.0f, 0.0f);
}

/**
 * Sets the assigned Npad id.
 * @param npadId Npad id
 */
void NpadController::setNpadId(s32 npadId) {
    mNpadId = npadId;
}

/**
 * Sets whether the Npad is connected.
 * @param isConnected whether it is connected
 */
void NpadController::setIsConnected(bool isConnected) {
    mIsConnected = isConnected;
}

/**
 * Sets the number of six-axis sensors.
 * @param num number of sensors
 */
void NpadController::setSixAxisSensorNum(s32 num) {
    mSixAxisSensorNum = num;
}

/**
 * Sets the sampling number of the last input.
 * @param samplingNumber sampling number
 */
void NpadController::setSamplingNumber(s64 samplingNumber) {
    mSamplingNumber = samplingNumber;
}

/**
 * Sets the Npad style.
 * @param style style
 */
void NpadController::setStyle(sead::NinJoyNpadDevice::Style style) {
    mStyle = style;
}

/**
 * Gathers and applies the input of the assigned Npad.
 */
void NpadController::calcImpl_() {
    gatherInput();
    applyInput();
}
}  // namespace al

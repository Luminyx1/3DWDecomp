#include "Project/Controller/PadTouchDevice.hpp"

#include <controller/seadControllerMgr.h>
#include <cstring>

namespace al {
/**
 * Creates the touch screen device and initializes the touch screen.
 * @param pMgr controller manager
 */
PadTouchDevice::PadTouchDevice(sead::ControllerMgr* pMgr) : sead::ControlDevice(pMgr) {
    std::memset(&mTouchScreenState, 0, sizeof(mTouchScreenState));
    nn::hid::InitializeTouchScreen();
    mId = sead::ControllerDefine::cDevice_PadTouch;
}

/**
 * Destroys the touch screen device.
 */
PadTouchDevice::~PadTouchDevice() = default;

/**
 * Reads the current touch screen state.
 */
void PadTouchDevice::calc() {
    nn::hid::GetTouchScreenState(&mTouchScreenState);
}

/**
 * Creates the touch screen controller.
 * @param pMgr controller manager
 */
PadTouchController::PadTouchController(sead::ControllerMgr* pMgr) : sead::Controller(pMgr) {
    mTouchScreenState.mCount = 0;
    mId = sead::ControllerDefine::cController_PadTouch;
}

/**
 * Destroys the touch screen controller.
 */
PadTouchController::~PadTouchController() = default;

/**
 * Updates the controller from the touch screen.
 */
void PadTouchController::calcImpl_() {
    gatherInput();
    applyInput();
}

/**
 * Copies the touch screen state from the touch screen device.
 * @return whether the device exists
 */
bool PadTouchController::gatherInput() {
    auto* device = static_cast<PadTouchDevice*>(
        getMgr()->getControlDevice(sead::ControllerDefine::cDevice_PadTouch));
    if (!device) {
        return false;
    }

    const nn::hid::TouchScreenState<1>* state = &device->getTouchScreenState();
    std::memmove(&mTouchScreenState, state, sizeof(mTouchScreenState));
    return true;
}

/**
 * Applies the touch screen state to the touch button and the pointer.
 */
void PadTouchController::applyInput() {
    bool isTouch = mTouchScreenState.mCount != 0;
    mPadHold.changeBit(cPadIdx_Touch, isTouch);
    setPointerWithBound_(isTouch, true,
                         sead::Vector2f(mTouchScreenState.mTouches[0].mX,
                                        mTouchScreenState.mTouches[0].mY));
}
}  // namespace al

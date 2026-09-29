#include "Project/Camera/Holder/SimpleCameraInput.hpp"

#include "Project/Controller/InputFunction.hpp"

namespace al {
/**
 * @brief Creates an input reading the given controller port.
 * @param port The controller port, or a negative value to use the main controller.
 */
SimpleCameraInput::SimpleCameraInput(s32 port) : mPort(port) {
    if (port < 0) {
        mPort = getMainControllerPort();
    }
}

/** @brief Tracks a short press of L, which resets the camera when L is released within ten frames. */
void SimpleCameraInput::updateInput() {
    if (isPadTypeJoySingle(mPort)) {
        return;
    }

    if (isPadHoldR(mPort)) {
        mHoldLFrame = -1;
        return;
    }

    if (isPadTriggerL(mPort)) {
        mHoldLFrame = 0;
        return;
    }

    if (mHoldLFrame < 0) {
        return;
    }

    if (isPadHoldL(mPort)) {
        mHoldLFrame = mHoldLFrame > 9 ? -1 : mHoldLFrame + 1;
    }
    else if (isPadReleaseL(mPort)) {
        mHoldLFrame = 10;
    }
    else {
        mHoldLFrame = -1;
    }
}

/**
 * @brief Calculates the stick input that rotates the camera.
 * @param pInputStick Where the stick input is written; zero while the input is disabled.
 */
void SimpleCameraInput::calcInputStick(sead::Vector2f* pInputStick) const {
    pInputStick->set(0.0f, 0.0f);
    if (mIsDisableInput) {
        return;
    }

    if (isPadTypeJoySingle(mPort)) {
        if (isPadHoldA(mPort)) {
            pInputStick->set(getLeftStick(mPort));
        }
    }
    else {
        pInputStick->set(getRightStick(mPort));
    }
}

/**
 * @brief Checks whether the camera reset was requested.
 * @return True on a short press of L, or on a press of the stick for a single Joy-Con.
 */
bool SimpleCameraInput::isTriggerReset() const {
    if (isPadTypeJoySingle(mPort)) {
        return isPadTriggerPressLeftStick(mPort);
    }
    return mHoldLFrame == 10;
}

/**
 * @brief Checks whether a zoom button is held.
 * @return True while ZL or ZR is held.
 */
bool SimpleCameraInput::isHoldZoom() const {
    return isPadHoldZL(mPort) || isPadHoldZR(mPort);
}

/**
 * @brief Checks whether the snapshot zoom in button is held.
 * @return True while X is held.
 */
bool SimpleCameraInput::isHoldSnapShotZoomIn() const {
    return isPadHoldX(mPort);
}

/**
 * @brief Checks whether the snapshot zoom out button is held.
 * @return True while A is held.
 */
bool SimpleCameraInput::isHoldSnapShotZoomOut() const {
    return isPadHoldA(mPort);
}

/**
 * @brief Checks whether the snapshot roll left button is held.
 * @return True while ZL is held.
 */
bool SimpleCameraInput::isHoldSnapShotRollLeft() const {
    return isPadHoldZL(mPort);
}

/**
 * @brief Checks whether the snapshot roll right button is held.
 * @return True while ZR is held.
 */
bool SimpleCameraInput::isHoldSnapShotRollRight() const {
    return isPadHoldZR(mPort);
}

/**
 * @brief Reads the stick that moves the snapshot camera.
 * @param pMoveStick Where the left stick of the main controller is written if it is tilted.
 * @return True if the stick is tilted.
 */
bool SimpleCameraInput::tryCalcSnapShotMoveStick(sead::Vector2f* pMoveStick) const {
    sead::Vector2f stick = getLeftStick(getMainControllerPort());
    if (stick.x * stick.x + stick.y * stick.y < 0.001) {
        return false;
    }
    *pMoveStick = stick;
    return true;
}
}  // namespace al

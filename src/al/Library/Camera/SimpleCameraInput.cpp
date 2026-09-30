#include "Library/Camera/SimpleCameraInput.hpp"

#include "Library/Controller/InputFunction.hpp"

namespace al {

SimpleCameraInput::SimpleCameraInput(s32 port) : mPort(port) {
    if (mPort < 0) {
        mPort = getMainControllerPort();
    }
}

void SimpleCameraInput::updateInput() {
    if (isPadTypeJoySingle(mPort)) {
        return;
    }

    if (isPadHoldR(mPort)) {
        mResetFrame = -1;
        return;
    }

    if (isPadTriggerL(mPort)) {
        mResetFrame = 0;
        return;
    }

    if (mResetFrame < 0) {
        return;
    }

    if (isPadHoldL(mPort)) {
        mResetFrame = mResetFrame > 9 ? -1 : mResetFrame + 1;
    } else if (isPadReleaseL(mPort)) {
        mResetFrame = 10;
    } else {
        mResetFrame = -1;
    }
}

void SimpleCameraInput::calcInputStick(sead::Vector2f* pStick) const {
    *pStick = {0.0f, 0.0f};
    if (mIsDisableInput) {
        return;
    }

    if (isPadTypeJoySingle(mPort)) {
        if (isPadHoldA(mPort)) {
            pStick->set(getLeftStick(mPort));
        }
    } else {
        pStick->set(getRightStick(mPort));
    }
}

bool SimpleCameraInput::isTriggerReset() const {
    if (isPadTypeJoySingle(mPort)) {
        return isPadTriggerPressLeftStick(mPort);
    }

    return mResetFrame == 10;
}

bool SimpleCameraInput::isHoldZoom() const {
    return isPadHoldZL(mPort) || isPadHoldZR(mPort);
}

bool SimpleCameraInput::isHoldSnapShotZoomIn() const {
    return isPadHoldX(mPort);
}

bool SimpleCameraInput::isHoldSnapShotZoomOut() const {
    return isPadHoldA(mPort);
}

bool SimpleCameraInput::isHoldSnapShotRollLeft() const {
    return isPadHoldZL(mPort);
}

bool SimpleCameraInput::isHoldSnapShotRollRight() const {
    return isPadHoldZR(mPort);
}

bool SimpleCameraInput::tryCalcSnapShotMoveStick(sead::Vector2f* pStick) const {
    sead::Vector2f stick = getLeftStick(getMainControllerPort());

    if (stick.squaredLength() < 0.001) {
        return false;
    }

    pStick->set(stick);
    return true;
}

}  // namespace al

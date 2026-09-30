#include "Library/Controller/GamePadSystem.hpp"

#include <controller/nin/seadNinJoyNpadDevice.h>
#include <controller/seadControllerMgr.h>
#include <nn/hid.h>

#include "Library/Controller/InputFunction.hpp"
#include "Library/Controller/NpadController.hpp"
#include "Library/Math/MathUtil.hpp"

namespace al {
namespace {
sead::NinJoyNpadDevice* getNpadDevice() {
    return sead::ControllerMgr::instance()->getControlDeviceAs<sead::NinJoyNpadDevice*>();
}

nn::hid::NpadStyleSet makeStyleSet(bool isFullKey, bool isHandheld, bool isJoyDual, bool isJoyLeft,
                                   bool isJoyRight) {
    nn::hid::NpadStyleSet styleSet;
    styleSet.Reset();
    styleSet.Set(static_cast<s32>(nn::hid::NpadStyleTag::NpadStyleFullKey), isFullKey);
    styleSet.Set(static_cast<s32>(nn::hid::NpadStyleTag::NpadStyleHandheld), isHandheld);
    styleSet.Set(static_cast<s32>(nn::hid::NpadStyleTag::NpadStyleJoyDual), isJoyDual);
    styleSet.Set(static_cast<s32>(nn::hid::NpadStyleTag::NpadStyleJoyLeft), isJoyLeft);
    styleSet.Set(static_cast<s32>(nn::hid::NpadStyleTag::NpadStyleJoyRight), isJoyRight);
    return styleSet;
}

NpadController* getNpadController(s32 port) {
    return static_cast<NpadController*>(sead::ControllerMgr::instance()->getController(port));
}
}  // namespace

/**
 * Constructs the game pad system.
 * @param isSinglePlay whether to start in single play mode
 */
GamePadSystem::GamePadSystem(bool isSinglePlay) {
    mPadNames.tryAllocBuffer(4, nullptr);
    mPadNames[0] = sead::WSafeString::cEmptyString;
    mPadNames[1] = sead::WSafeString::cEmptyString;
    mPadNames[2] = sead::WSafeString::cEmptyString;
    mPadNames[3] = sead::WSafeString::cEmptyString;
    for (s32 i = 0; i < 9; i++) {
        mPadStyles[i] = 5;
        mPadConnectStates[i] = 0;
    }

    if (isSinglePlay) {
        changeSinglePlayMode(true);
    } else {
        changeMultiPlayMode(4, 1);
    }
}

/**
 * Changes to single play mode.
 * @param isAnyController whether any controller may be used
 */
void GamePadSystem::changeSinglePlayMode(bool isAnyController) {
    mMaxPlayerNum = 1;
    mMinPlayerNum = 1;
    sead::NinJoyNpadDevice* device = getNpadDevice();
    if (!isAnyController) {
        return;
    }

    device->setNpadJoyHoldType(nn::hid::NpadJoyHoldType(1));
    device->setSupportedNpadStyleSet(makeStyleSet(true, true, true, false, false));
    getNpadController(getPlayerControllerPort(0))->setAnyControllerMode();
}

/**
 * Changes to multi play mode.
 * @param maxPlayerNum maximum number of players
 * @param minPlayerNum minimum number of players
 */
void GamePadSystem::changeMultiPlayMode(s32 maxPlayerNum, s32 minPlayerNum) {
    mMaxPlayerNum = maxPlayerNum;
    mMinPlayerNum = minPlayerNum;
    sead::NinJoyNpadDevice* device = getNpadDevice();
    device->setNpadIdUpdateNum(maxPlayerNum);
    device->setNpadJoyHoldType(nn::hid::NpadJoyHoldType(1));
    if (!mIsAllowHandheld) {
        device->setSupportedNpadStyleSet(makeStyleSet(true, false, true, true, true));
        for (s32 i = 0; i < maxPlayerNum; i++) {
            getNpadController(getPlayerControllerPort(i))->setIndexControllerMode(i);
        }

        return;
    }

    if (mMaxPlayerNum == 1) {
        device->setSupportedNpadStyleSet(makeStyleSet(true, true, true, false, false));
    } else {
        device->setSupportedNpadStyleSet(makeStyleSet(true, true, true, true, true));
    }

    for (s32 i = 0; i < maxPlayerNum; i++) {
        NpadController* controller = getNpadController(getPlayerControllerPort(i));
        if (i == 0) {
            controller->setAnyControllerMode();
        } else {
            controller->setIndexControllerMode(i);
        }
    }
}

/**
 * Disables the controller applet.
 * @param isDisable whether to disable it
 */
void GamePadSystem::disableControllerApplet(bool isDisable) {
    if (mDelegate != nullptr) {
        mDelegate->disableControllerApplet(isDisable);
    }
}

/**
 * Disables the controller connect checker.
 * @param isDisable whether to disable it
 */
void GamePadSystem::disableControllerConnectChecker(bool isDisable) {
    if (mDelegate != nullptr) {
        mDelegate->disableControllerConnectChecker(isDisable);
    }
}

/**
 * Sets whether handheld play is allowed.
 * @param isAllow whether handheld play is allowed
 */
void GamePadSystem::setIsAllowHandheld(bool isAllow) {
    if (!mIsEnableAutoHandheld) {
        return;
    }

    mIsAllowHandheld = isAllow;
    changeMultiPlayMode(mMaxPlayerNum, mMinPlayerNum);
}

/**
 * Sets the number of supported npads.
 * @param num number of npads
 */
void GamePadSystem::setMaxNpadNum(s32 num) {
    num = sead::Mathi::clamp(num, 1, 9);
    u32 npadIds[9];
    for (s32 i = 0; i < num; i++) {
        npadIds[i] = i;
    }

    nn::hid::SetSupportedNpadIdType(npadIds, num);
}

/**
 * Assigns single Joy-Cons to their own npads.
 */
void GamePadSystem::initSingleJoycon() {
    sead::NinJoyNpadDevice* device = getNpadDevice();
    s32 num = device->getNpadIdUpdateNum();
    for (s32 i = 0; i <= num; i++) {
        s32 index = i == num ? 8 : i;
        if (device->getNpadStyleTag(index) != nn::hid::NpadStyleTag::NpadStyleJoyDual) {
            continue;
        }

        const nn::hid::NpadAttributeSet& attributes =
            device->getNpadState(index).mStates[0].mAttributes;
        bool isLeftConnected =
            attributes.Test(static_cast<s32>(nn::hid::NpadAttribute::IsLeftConnected));
        bool isRightConnected =
            attributes.Test(static_cast<s32>(nn::hid::NpadAttribute::IsRightConnected));
        if (isLeftConnected) {
            if (!isRightConnected) {
                device->setNpadJoyAssignmentModeSingle(index);
            }
        } else if (isRightConnected) {
            device->setNpadJoyAssignmentModeSingle(index);
        }
    }
}

/**
 * Checks whether the game should be paused because controllers are disconnected.
 * @return whether the disconnect frame count was exceeded
 */
bool GamePadSystem::isDisconnectPlayable() const {
    if (mInvalidateDisconnectFrame > 0) {
        return false;
    }

    return mDisconnectFrame > mDisconnectFrameMax;
}

/**
 * Gets the name of a pad.
 * @param index pad index
 * @return pad name
 */
const sead::WSafeString& GamePadSystem::getPadName(u8 index) const {
    return mPadNames[index];
}

/**
 * Gets the play style of a player controller.
 * @param index player index
 * @return play style
 */
s32 GamePadSystem::getPadPlayStyle(u8 index) const {
    s32 port = getPlayerControllerPort(index);
    return static_cast<NpadController*>(sead::ControllerMgr::instance()->getControllerUnsafe(port))
        ->getStyle();
}

/**
 * Updates the pad states and the disconnect frame counter.
 */
void GamePadSystem::update() {
    sead::NinJoyNpadDevice* device = getNpadDevice();
    s32 num = device->getNpadIdUpdateNum();
    mIsChangedPadState = false;
    bool isAllowHandheld = true;
    for (s32 i = 0; i <= num; i++) {
        s32 index = i == num ? 8 : i;
        s32 style = static_cast<s32>(device->getNpadStyleTag(index));
        if (style != mPadStyles[i]) {
            mIsChangedPadState = true;
        }

        mPadStyles[i] = style;
        s32 connectState = isPadConnected(i) ? 2 : isPadWaitingConnect(i);
        if (mPadConnectStates[i] != connectState) {
            mIsChangedPadState = true;
        }

        mPadConnectStates[i] = connectState;

        if (style == static_cast<s32>(nn::hid::NpadStyleTag::NpadStyleFullKey) ||
            style == static_cast<s32>(nn::hid::NpadStyleTag::NpadStyleJoyLeft) ||
            style == static_cast<s32>(nn::hid::NpadStyleTag::NpadStyleJoyRight)) {
            if (i != 0 && device->getNpadState(index).mStates[0].mAttributes.Test(
                              static_cast<s32>(nn::hid::NpadAttribute::IsConnected))) {
                isAllowHandheld = false;
            }
        } else if (style == static_cast<s32>(nn::hid::NpadStyleTag::NpadStyleJoyDual)) {
            const nn::hid::NpadAttributeSet& attributes =
                device->getNpadState(index).mStates[0].mAttributes;
            bool isConnected =
                attributes.Test(static_cast<s32>(nn::hid::NpadAttribute::IsLeftConnected)) ||
                attributes.Test(static_cast<s32>(nn::hid::NpadAttribute::IsRightConnected));
            isAllowHandheld &= i == 0 || !isConnected;
        }

        if (!isAllowHandheld) {
            break;
        }
    }

    if (mIsEnableAutoHandheld && isAllowHandheld != mIsAllowHandheld) {
        sead::NinJoyNpadDevice* npadDevice = getNpadDevice();
        if (isAllowHandheld) {
            npadDevice->setSupportedNpadStyleSet(makeStyleSet(true, true, true, true, true));
        } else {
            npadDevice->setSupportedNpadStyleSet(makeStyleSet(true, false, true, true, true));
        }

        npadDevice->setNpadJoyHoldType(nn::hid::NpadJoyHoldType(1));
        setIsAllowHandheld(isAllowHandheld);
    }

    mInvalidateDisconnectFrame = converge(mInvalidateDisconnectFrame, 0, 1);
    if (isDisconnectPlayableImpl()) {
        mDisconnectFrame = converge(mDisconnectFrame, 3600, 1);
    } else {
        mDisconnectFrame = 0;
    }
}

/**
 * Checks whether too few player controllers are connected.
 * @return whether the controllers count as disconnected
 */
bool GamePadSystem::isDisconnectPlayableImpl() const {
    s32 connectedNum = 0;
    for (s32 i = 0; i < mMaxPlayerNum; i++) {
        if (isPadConnected(getPlayerControllerPort(i))) {
            connectedNum++;
        }
    }

    if (connectedNum < mMinPlayerNum) {
        return true;
    }

    if (mDisconnectCallback == nullptr) {
        return false;
    }

    return mDisconnectCallback(this, mDisconnectCallbackUserData);
}

/**
 * Sets the frames until disconnected controllers pause the game.
 * @param frame number of frames
 */
void GamePadSystem::setDisconnectFrame(s32 frame) {
    mDisconnectFrameMax = frame;
}

/**
 * Makes disconnected controllers pause the game immediately.
 * @param isForce whether to force it
 */
void GamePadSystem::forceImmediateDisconnect(bool isForce) {
    mDisconnectFrame = mDisconnectFrameMax;
    mIsForceImmediateDisconnect = isForce;
}

/**
 * Sets the frames during which disconnects are ignored.
 * @param frame number of frames
 */
void GamePadSystem::setInvalidateDisconnectFrame(s32 frame) {
    mInvalidateDisconnectFrame = frame;
}

/**
 * Sets the name of a pad.
 * @param index pad index
 * @param rName pad name
 */
void GamePadSystem::setPadName(u8 index, const sead::WSafeString& rName) {
    mPadNames[index].copy(rName);
}

/**
 * Calls the disconnect controller handler.
 */
void GamePadSystem::callDisconnectController() {
    if (mDelegate != nullptr) {
        mDelegate->callDisconnectController();
    }
}

/**
 * Changes to the play mode of the top menu.
 */
void GamePadSystem::changeTopMenuPlayMode() {
    mMaxPlayerNum = 1;
    mMinPlayerNum = 1;
    mIsEnableAutoHandheld = false;
    mIsAllowHandheld = true;
    sead::NinJoyNpadDevice* device = getNpadDevice();
    device->setNpadJoyHoldType(nn::hid::NpadJoyHoldType(1));
    device->setSupportedNpadStyleSet(makeStyleSet(true, true, true, true, true));
    getNpadController(getPlayerControllerPort(0))->setAnyControllerMode();
}

/**
 * Changes the assist mode.
 * @param isAssist whether a second player assists
 * @param isForceDisconnect whether disconnects pause the game immediately
 */
void GamePadSystem::setAssistMode(bool isAssist, bool isForceDisconnect) {
    sead::NinJoyNpadDevice* device = getNpadDevice();
    device->setNpadJoyHoldType(nn::hid::NpadJoyHoldType(1));
    if (isAssist) {
        mMaxPlayerNum = 2;
        mMinPlayerNum = 2;
        device->setNpadIdUpdateNum(2);
        device->setSupportedNpadStyleSet(makeStyleSet(true, true, true, true, true));
        for (s32 i = 0; i < mMaxPlayerNum; i++) {
            getNpadController(getPlayerControllerPort(i))->setIndexControllerMode(i);
        }
    } else {
        mMaxPlayerNum = 1;
        mMinPlayerNum = 1;
        device->setNpadIdUpdateNum(1);
        device->setSupportedNpadStyleSet(makeStyleSet(true, true, true, false, false));
        getNpadController(getPlayerControllerPort(0))->setAnyControllerMode();
    }

    if (isForceDisconnect) {
        mIsForceImmediateDisconnect = true;
    }
}

/**
 * Sets the software keyboard.
 * @param pCancel cancel handler
 */
void GamePadSystem::setSoftwareKeyboard(IUseCancel* pCancel) {
    if (mDelegate != nullptr) {
        mDelegate->setSoftwareKeyboard(pCancel);
    }
}
}  // namespace al

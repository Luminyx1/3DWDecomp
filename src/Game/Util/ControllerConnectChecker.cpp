#include "Util/ControllerConnectChecker.hpp"

#include <controller/nin/seadNinJoyNpadDevice.h>
#include <controller/seadControllerMgr.h>
#include <nn/hid.h>

#include "Library/Application/ApplicationMessageReceiver.hpp"
#include "Library/Controller/IUseCancel.hpp"
#include "Library/Controller/InputFunction.hpp"
#include "Library/Controller/NpadController.hpp"
#include "System/Data/SingleModeDataFunction.hpp"
#include "System/GameDataFunction.hpp"
#include "System/GameDataHolderAccessor.hpp"
#include "System/GameDataHolderWriter.hpp"
#include "Util/ControlUserUtil.hpp"

namespace {
/**
 * Gets the Npad control device.
 * @return the Npad device, or nullptr if none is registered
 */
sead::NinJoyNpadDevice* getNpadDevice() {
    return sead::ControllerMgr::instance()->getControlDeviceAs<sead::NinJoyNpadDevice*>();
}

/**
 * Gets the Npad controller of a port.
 * @param port controller port
 * @return the Npad controller, or nullptr if the port has none
 */
al::NpadController* getNpadController(s32 port) {
    return sead::DynamicCast<al::NpadController>(
        sead::ControllerMgr::instance()->getController(port));
}

/**
 * Builds a set of supported Npad styles.
 * @param isFullKey whether the Pro Controller style is supported
 * @param isHandheld whether the handheld style is supported
 * @param isJoyDual whether the dual Joy-Con style is supported
 * @param isJoyLeft whether the single left Joy-Con style is supported
 * @param isJoyRight whether the single right Joy-Con style is supported
 * @return the style set
 */
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
}  // namespace

/**
 * Constructs the controller connect checker and registers it as the game pad system's delegate.
 * @param pHolder game data holder
 * @param pMessageReceiver application message receiver (operation mode / resume state)
 * @param pGamePadSystem game pad system to watch
 */
ControllerConnectChecker::ControllerConnectChecker(
    GameDataHolder* pHolder, const al::ApplicationMessageReceiver* pMessageReceiver,
    al::GamePadSystem* pGamePadSystem)
    : mGameDataHolder(pHolder), mMessageReceiver(pMessageReceiver),
      mGamePadSystem(pGamePadSystem),
      mIsConsoleMode(pMessageReceiver->getCachedOperationMode() ==
                     nn::oe::OperationMode_Docked) {
    pGamePadSystem->setDelegate(this);
}

/**
 * Handles a disconnect request from the game pad system (does nothing).
 * @return always false
 */
bool ControllerConnectChecker::disconnect() {
    return false;
}

/**
 * Sets the object that is cancelled before the controller applet is shown.
 * @param pCancelUser object to cancel (e.g. a software keyboard), or nullptr
 */
void ControllerConnectChecker::setCancelUser(al::IUseCancel* pCancelUser) {
    mCancelUser = pCancelUser;
}

/**
 * Checks for disconnected controllers and shows the controller support applet when needed.
 */
void ControllerConnectChecker::update() {
    if (mIsDisabled) {
        return;
    }

    bool isPrevConsoleMode = mIsConsoleMode;
    mIsConsoleMode = mMessageReceiver->getCachedOperationMode() == nn::oe::OperationMode_Docked;

    if (mWaitFrame != 0) {
        mWaitFrame--;
    }

    bool isDisconnect = mGamePadSystem->isDisconnectPlayable();
    bool isForceDisconnect = false;

    if (mGamePadSystem->isForceImmediateDisconnect()) {
        mGamePadSystem->resetForceImmediateDisconnect();
        isForceDisconnect = true;
        isDisconnect = true;
    }

    if (isDisconnect && mMessageReceiver->isResumed()) {
        mWaitFrame = 0;
    }

    if (mIsConsoleMode != isPrevConsoleMode) {
        u32 waitFrame = mIsConsoleMode ? 10 : 2;

        if (waitFrame > mWaitFrame) {
            mWaitFrame = waitFrame;
        }
    }

    if (isDisconnect && !isForceDisconnect && !mGamePadSystem->isAllowHandheld()) {
        mGamePadSystem->setIsAllowHandheld(true);
    }

    if (mWaitFrame != 0 || !isDisconnect) {
        return;
    }

    s32 validNpadNum = 0;

    for (s32 i = 0; i < 5; i++) {
        al::NpadController* controller = getNpadController(i);

        if (controller != nullptr) {
            validNpadNum += controller->isValidNpadId();
        }
    }

    if (!isForceDisconnect &&
        SingleModeDataFunction::getIs2PAssistMode(GameDataHolderAccessor(mGameDataHolder))) {
        mGamePadSystem->setAssistMode(false, false);
        SingleModeDataFunction::setIs2PAssistMode(GameDataHolderWriter(mGameDataHolder), false);
    }

    nn::hid::NpadStyleSet styleSet = nn::hid::GetSupportedNpadStyleSet();

    u8 playerCount;
    bool isSuccess;

    while (true) {
        s32 minPlayerNum = mGamePadSystem->getMinPlayerNum();

        if (validNpadNum == 0) {
            sead::NinJoyNpadDevice* device = getNpadDevice();
            device->setNpadJoyHoldType(nn::hid::NpadJoyHoldType(1));

            if (mGamePadSystem->isEnableAutoHandheld()) {
                if (mGamePadSystem->getMaxPlayerNum() == 1) {
                    device->setSupportedNpadStyleSet(makeStyleSet(true, true, true, false, false));
                } else {
                    device->setSupportedNpadStyleSet(makeStyleSet(true, true, true, true, true));
                }
            }
        }

        playerCount = 0;
        isSuccess = true;

        if (!mIsAppletDisabled) {
            isSuccess = showControllerSupportApplet(playerCount);
        }

        if (isSuccess && (minPlayerNum != 2 || playerCount > 1)) {
            sead::ControllerMgr::instance()->calc();

            if (al::getMainControllerPort() != -1) {
                sead::NinJoyNpadDevice* device = getNpadDevice();
                device->setNpadJoyHoldType(nn::hid::NpadJoyHoldType(1));
                device->setSupportedNpadStyleSet(styleSet);
                return;
            }
        } else {
            break;
        }

        mWaitFrame = 0;
    }

    bool isSingleMode = GameDataFunction::isSingleMode(GameDataHolderAccessor(mGameDataHolder));

    if (isForceDisconnect && isSingleMode && !isSuccess) {
        bool is2PAssistMode = mGamePadSystem->is2PAssistMode();
        rc::set2PAssistMode(is2PAssistMode, false);
        SingleModeDataFunction::setIs2PAssistMode(GameDataHolderWriter(mGameDataHolder),
                                                  is2PAssistMode);
        return;
    }

    sead::ControllerMgr::instance()->calc();
    sead::NinJoyNpadDevice* device = getNpadDevice();

    if (playerCount != 0) {
        device->setNpadJoyHoldType(nn::hid::NpadJoyHoldType(1));
        device->setSupportedNpadStyleSet(styleSet);
    } else if (mGamePadSystem->isEnableAutoHandheld()) {
        mGamePadSystem->setIsAllowHandheld(true);

        if (!styleSet.Test(static_cast<s32>(nn::hid::NpadStyleTag::NpadStyleHandheld))) {
            device->setNpadJoyHoldType(nn::hid::NpadJoyHoldType(1));
            device->setSupportedNpadStyleSet(makeStyleSet(true, true, true, true, true));
        }
    }

    mWaitFrame = 0;
}

/**
 * Shows the controller support applet for the current player range.
 * @param rPlayerCount receives the number of players selected in the applet
 * @return whether the applet finished successfully
 */
bool ControllerConnectChecker::showControllerSupportApplet(u8& rPlayerCount) const {
    bool isSingleAllowed =
        mGamePadSystem->isAllowHandheld() || mGamePadSystem->getMinPlayerNum() < 2;

    nn::hid::ControllerSupportArg arg;
    arg.SetDefault();
    arg.mMinPlayerCount = mGamePadSystem->getMinPlayerNum();
    arg.mMaxPlayerCount = mGamePadSystem->getMaxPlayerNum();
    arg.mTakeOverConnection = true;
    arg.mLeftJustify = false;
    arg.mPermitJoyconDual = true;
    bool isHandheldSingle = mIsConsoleMode ? false : isSingleAllowed;
    arg.mSingleMode = isHandheldSingle & (arg.mMaxPlayerCount < 2);
    arg.mUseColors = false;

    if (mCancelUser != nullptr) {
        mCancelUser->cancel();
    }

    nn::hid::ControllerSupportResultInfo resultInfo;
    bool isSuccess = al::tryCallControllerApplet(mGamePadSystem, &arg, &resultInfo, true);
    rPlayerCount = resultInfo.mPlayerCount;
    return isSuccess;
}

/**
 * Disconnects every connected single Joy-Con used by a player.
 */
void ControllerConnectChecker::disconnectControllers() const {
    sead::ControllerMgr* controllerMgr = sead::ControllerMgr::instance();
    sead::NinJoyNpadDevice* device =
        controllerMgr->getControlDeviceAs<sead::NinJoyNpadDevice*>();
    s32 maxPlayerNum = mGamePadSystem->getMaxPlayerNum();

    for (s32 i = 0; i < maxPlayerNum; i++) {
        s32 port = al::getPlayerControllerPort(i);

        if (!al::isPadConnected(port)) {
            continue;
        }

        al::NpadController* controller =
            sead::DynamicCast<al::NpadController>(controllerMgr->getController(port));

        if (controller == nullptr) {
            continue;
        }

        sead::NinJoyNpadDevice::Style style = controller->getStyle();

        if (style != sead::NinJoyNpadDevice::cStyle_JoyLeft &&
            style != sead::NinJoyNpadDevice::cStyle_JoyRight) {
            continue;
        }

        if (controller->getNpadId() == -1) {
            continue;
        }

        device->disconnectNpad(controller->getNpadId());
    }
}

/**
 * Sets whether the controller support applet is disabled.
 * @param isDisabled whether to disable the applet
 */
void ControllerConnectChecker::setAppletDisabled(bool isDisabled) {
    mIsAppletDisabled = isDisabled;
}

/**
 * Sets whether the connect checker is disabled.
 * @param isDisabled whether to disable the checker
 */
void ControllerConnectChecker::setDisabled(bool isDisabled) {
    mIsDisabled = isDisabled;
}

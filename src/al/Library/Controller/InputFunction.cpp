#include "Library/Controller/InputFunction.hpp"

#include <controller/seadController.h>
#include <controller/seadAccelerometerAddon.h>
#include <controller/seadControllerMgr.h>

#include "Library/Controller/NpadController.hpp"
#include "Library/Controller/PadGyroAddon.hpp"
#include "Library/Controller/PadReplayFunction.hpp"
#include "Library/Controller/ReplayController.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Screen/ScreenFunction.hpp"
#include "Project/Controller/JoyPadAccelerometerAddon.hpp"
#include "Project/Controller/PadUiKeyInputAddon.hpp"

namespace al {
namespace {
bool sIsInGetMainControllerPortCallback = false;
GetMainControllerPortCallbackFn sGetMainControllerPortCallback = nullptr;

inline sead::ControllerBase* getControllerDirect(s32 port) {
    if (isValidReplayController(port)) {
        return getReplayController(port);
    }

    return sead::ControllerMgr::instance()->getController(port);
}

inline sead::ControllerBase* getController(s32 port) {
    if (port == -1) {
        port = getMainControllerPort();
    }

    return getControllerDirect(port);
}

inline NpadController* tryGetNpadController(s32 port) {
    sead::ControllerBase* controller = getControllerDirect(port);

    if (isValidReplayController(port)) {
        controller = sead::DynamicCast<ReplayController>(controller)->getController();
    }

    return sead::DynamicCast<NpadController>(controller);
}

inline PadGyroAddon* tryGetGyroAddon(s32 port, s32 index) {
    if (port == -1) {
        port = getMainControllerPort();
    }

    sead::Controller* controller = sead::ControllerMgr::instance()->getController(port);
    return controller->getAddonByOrderAs<PadGyroAddon*>(index);
}

bool isPadStyle(s32 port, sead::NinJoyNpadDevice::Style style) {
    return tryGetNpadController(port)->getStyle() == style;
}

inline PadUiKeyInputAddon* getUiKeyInputAddon(s32 port) {
    if (port == -1) {
        port = getMainControllerPort();
    }

    sead::Controller* controller = sead::ControllerMgr::instance()->getControllerUnsafe(port);
    return controller->getAddonAs<PadUiKeyInputAddon*>();
}
}  // namespace

/**
 * Returns the port of the right single Joy-Con.
 * @return controller port
 */
s32 getJoyPadSingleRightPort() {
    return 1;
}

/**
 * Returns the port of the left single Joy-Con.
 * @return controller port
 */
s32 getJoyPadSingleLeftPort() {
    return 2;
}

/**
 * Returns the port of the first Npad.
 * @return controller port
 */
s32 getJoyPadDoublePort() {
    sead::ControllerMgr* mgr = sead::ControllerMgr::instance();
    return mgr->findControllerPort(
        mgr->getControllerByOrder(sead::ControllerDefine::cController_Npad, 0));
}

/**
 * Returns the port of a touch panel.
 * @param index touch panel index
 * @return controller port, or -1
 */
s32 getTouchPanelPort(s32 index) {
    sead::ControllerMgr* mgr = sead::ControllerMgr::instance();
    sead::Controller* controller =
        mgr->getControllerByOrder(sead::ControllerDefine::cController_PadTouch, index);
    if (controller == nullptr) {
        return -1;
    }

    return mgr->findControllerPort(controller);
}

/**
 * Returns the number of controller ports.
 * @return number of ports
 */
s32 getMaxControllerPorts() {
    return 4;
}

/**
 * Returns the port of the main controller.
 * @return controller port
 */
s32 getMainControllerPort() {
    if (!sIsInGetMainControllerPortCallback && sGetMainControllerPortCallback) {
        sIsInGetMainControllerPortCallback = true;
        s32 port = sGetMainControllerPortCallback();
        sIsInGetMainControllerPortCallback = false;
        return port;
    }

    return getMainJoyPadDoublePort();
}

/**
 * Returns the port of a player's Npad.
 * @param playerIndex player index
 * @return controller port, or -1
 */
s32 getPlayerControllerPort(s32 playerIndex) {
    sead::ControllerMgr* mgr = sead::ControllerMgr::instance();
    sead::Controller* controller =
        mgr->getControllerByOrder(sead::ControllerDefine::cController_Npad, playerIndex);
    if (controller == nullptr) {
        return -1;
    }

    return mgr->findControllerPort(controller);
}

/**
 * Returns the port of the main Npad.
 * @return controller port
 */
s32 getMainJoyPadDoublePort() {
    sead::ControllerMgr* mgr = sead::ControllerMgr::instance();
    return mgr->findControllerPort(
        mgr->getControllerByOrder(sead::ControllerDefine::cController_Npad, 0));
}

/**
 * Returns the port of the main right single Joy-Con.
 * @return controller port
 */
s32 getMainJoyPadSingleRightPort() {
    return 1;
}

/**
 * Returns the port of the main left single Joy-Con.
 * @return controller port
 */
s32 getMainJoyPadSingleLeftPort() {
    return 2;
}

/**
 * Sets the callback that decides the main controller port.
 * @param pCallback new callback
 * @return the previous callback
 */
GetMainControllerPortCallbackFn setGetMainControllerPortCallbackFn(
    GetMainControllerPortCallbackFn pCallback) {
    GetMainControllerPortCallbackFn prev = sGetMainControllerPortCallback;
    sGetMainControllerPortCallback = pCallback;
    return prev;
}

/**
 * Checks whether buttons were pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @param mask button mask
 * @return whether any of the buttons was pressed
 */
bool isPadTrigger(s32 port, s32 mask) {
    return getController(port)->isTrig(mask);
}

/**
 * Sets the repeat parameters of buttons.
 * @param mask button mask
 * @param delay frames before the first repeat
 * @param pulse frames between repeats
 * @param port controller port, or -1 for the main controller
 */
void setPadRepeat(s32 mask, s32 delay, s32 pulse, s32 port) {
    getController(port)->setPadRepeat(mask, delay, pulse);
}

/**
 * Checks whether buttons are held.
 * @param port controller port, or -1 for the main controller
 * @param mask button mask
 * @return whether any of the buttons is held
 */
bool isPadHold(s32 port, s32 mask) {
    return getController(port)->isHold(mask);
}

/**
 * Returns the left stick position.
 * @param port controller port, or -1 for the main controller
 * @return left stick position
 */
const sead::Vector2f& getLeftStick(s32 port) {
    return getController(port)->getLeftStick();
}

/**
 * Returns the right stick position.
 * @param port controller port, or -1 for the main controller
 * @return right stick position
 */
const sead::Vector2f& getRightStick(s32 port) {
    return getController(port)->getRightStick();
}

/**
 * Checks whether a player's controller is assigned to an Npad id.
 * @param npadId Npad id to compare with
 * @param playerIndex player index
 * @return whether the ids are the same
 */
bool isSameNpadId(u32 npadId, s32 playerIndex) {
    s32 port = getPlayerControllerPort(playerIndex);

    if (port < 0) {
        return false;
    }

    NpadController* npad = tryGetNpadController(port);

    if (npad == nullptr) {
        return false;
    }

    s32 id = npad->getNpadId();

    if (npadId == 0x20 && id == 8) {
        return true;
    }

    return npad->getNpadId() == npadId;
}

/**
 * Checks whether the A button was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerA(s32 port) {
    return getController(port)->isTrig(1 << 0);
}

/**
 * Checks whether the B button was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerB(s32 port) {
    return getController(port)->isTrig(1 << 1);
}

/**
 * Checks whether the X button was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerX(s32 port) {
    return getController(port)->isTrig(1 << 3);
}

/**
 * Checks whether the Y button was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerY(s32 port) {
    return getController(port)->isTrig(1 << 4);
}

/**
 * Checks whether the ZL button was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerZL(s32 port) {
    return getController(port)->isTrig(1 << 2);
}

/**
 * Checks whether the ZR button was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerZR(s32 port) {
    return getController(port)->isTrig(1 << 5);
}

/**
 * Checks whether the L button was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerL(s32 port) {
    return getController(port)->isTrig(1 << 13);
}

/**
 * Checks whether the R button was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerR(s32 port) {
    return getController(port)->isTrig(1 << 14);
}

/**
 * Checks whether the 1 button was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTrigger1(s32 port) {
    return getController(port)->isTrig(1 << 7);
}

/**
 * Checks whether the 2 button was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTrigger2(s32 port) {
    return getController(port)->isTrig(1 << 6);
}

/**
 * Checks whether the Up button was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerUp(s32 port) {
    return getController(port)->isTrig(1 << 16);
}

/**
 * Checks whether the Down button was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerDown(s32 port) {
    return getController(port)->isTrig(1 << 17);
}

/**
 * Checks whether the Left button was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerLeft(s32 port) {
    return getController(port)->isTrig(1 << 18);
}

/**
 * Checks whether the Right button was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerRight(s32 port) {
    return getController(port)->isTrig(1 << 19);
}

/**
 * Checks whether a diagonal direction was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether the direction was pressed
 */
bool isPadTriggerLeftUp(s32 port) {
    return isPadHoldLeftUp(port) && getController(port)->isTrig(0x50000);
}

/**
 * Checks whether a diagonal direction was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether the direction was pressed
 */
bool isPadTriggerLeftDown(s32 port) {
    return isPadHoldLeftDown(port) && getController(port)->isTrig(0x60000);
}

/**
 * Checks whether a diagonal direction was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether the direction was pressed
 */
bool isPadTriggerRightUp(s32 port) {
    return isPadHoldRightUp(port) && getController(port)->isTrig(0x90000);
}

/**
 * Checks whether a diagonal direction was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether the direction was pressed
 */
bool isPadTriggerRightDown(s32 port) {
    return isPadHoldRightDown(port) && getController(port)->isTrig(0xA0000);
}

/**
 * Checks whether the Home button was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerHome(s32 port) {
    return getController(port)->isTrig(1 << 8);
}

/**
 * Checks whether the Start button was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerStart(s32 port) {
    return getController(port)->isTrig(1 << 11);
}

/**
 * Checks whether the Select button was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerSelect(s32 port) {
    return getController(port)->isTrig(1 << 12);
}

/**
 * Checks whether the Plus button was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerPlus(s32 port) {
    return getController(port)->isTrig(1 << 10);
}

/**
 * Checks whether the Minus button was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerMinus(s32 port) {
    return getController(port)->isTrig(1 << 9);
}

/**
 * Checks whether the Touch button was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerTouch(s32 port) {
    return getController(port)->isTrig(1 << 15);
}

/**
 * Checks whether the UpLeftStick direction was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerUpLeftStick(s32 port) {
    return getController(port)->isTrig(1 << 20);
}

/**
 * Checks whether the DownLeftStick direction was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerDownLeftStick(s32 port) {
    return getController(port)->isTrig(1 << 21);
}

/**
 * Checks whether the LeftLeftStick direction was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerLeftLeftStick(s32 port) {
    return getController(port)->isTrig(1 << 22);
}

/**
 * Checks whether the RightLeftStick direction was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerRightLeftStick(s32 port) {
    return getController(port)->isTrig(1 << 23);
}

/**
 * Checks whether the UpRightStick direction was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerUpRightStick(s32 port) {
    return getController(port)->isTrig(1 << 24);
}

/**
 * Checks whether the DownRightStick direction was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerDownRightStick(s32 port) {
    return getController(port)->isTrig(1 << 25);
}

/**
 * Checks whether the LeftRightStick direction was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerLeftRightStick(s32 port) {
    return getController(port)->isTrig(1 << 26);
}

/**
 * Checks whether the RightRightStick direction was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerRightRightStick(s32 port) {
    return getController(port)->isTrig(1 << 27);
}

/**
 * Checks whether any button was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerAny(s32 port) {
    return getController(port)->isTrig(0xFFF7FFF);
}

/**
 * Checks whether the left stick direction was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerLeftStick(s32 port) {
    return getController(port)->isTrig(0xF00000);
}

/**
 * Checks whether the right stick direction was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerRightStick(s32 port) {
    return getController(port)->isTrig(0xF000000);
}

/**
 * Checks whether the PressLeftStick direction was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerPressLeftStick(s32 port) {
    return getController(port)->isTrig(1 << 7);
}

/**
 * Checks whether the PressRightStick direction was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame
 */
bool isPadTriggerPressRightStick(s32 port) {
    return getController(port)->isTrig(1 << 6);
}

/**
 * Checks whether the PressLeftStick direction is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldPressLeftStick(s32 port) {
    return getController(port)->isHold(1 << 7);
}

/**
 * Checks whether the PressRightStick direction is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldPressRightStick(s32 port) {
    return getController(port)->isHold(1 << 6);
}

/**
 * Checks whether the A button was pressed this frame or repeats.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame or repeats
 */
bool isPadRepeatA(s32 port) {
    return getController(port)->isTrigWithRepeat(1 << 0);
}

/**
 * Checks whether the B button was pressed this frame or repeats.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame or repeats
 */
bool isPadRepeatB(s32 port) {
    return getController(port)->isTrigWithRepeat(1 << 1);
}

/**
 * Checks whether the X button was pressed this frame or repeats.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame or repeats
 */
bool isPadRepeatX(s32 port) {
    return getController(port)->isTrigWithRepeat(1 << 3);
}

/**
 * Checks whether the Y button was pressed this frame or repeats.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame or repeats
 */
bool isPadRepeatY(s32 port) {
    return getController(port)->isTrigWithRepeat(1 << 4);
}

/**
 * Checks whether the ZL button was pressed this frame or repeats.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame or repeats
 */
bool isPadRepeatZL(s32 port) {
    return getController(port)->isTrigWithRepeat(1 << 2);
}

/**
 * Checks whether the ZR button was pressed this frame or repeats.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame or repeats
 */
bool isPadRepeatZR(s32 port) {
    return getController(port)->isTrigWithRepeat(1 << 5);
}

/**
 * Checks whether the L button was pressed this frame or repeats.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame or repeats
 */
bool isPadRepeatL(s32 port) {
    return getController(port)->isTrigWithRepeat(1 << 13);
}

/**
 * Checks whether the R button was pressed this frame or repeats.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame or repeats
 */
bool isPadRepeatR(s32 port) {
    return getController(port)->isTrigWithRepeat(1 << 14);
}

/**
 * Checks whether the 1 button was pressed this frame or repeats.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame or repeats
 */
bool isPadRepeat1(s32 port) {
    return getController(port)->isTrigWithRepeat(1 << 7);
}

/**
 * Checks whether the 2 button was pressed this frame or repeats.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame or repeats
 */
bool isPadRepeat2(s32 port) {
    return getController(port)->isTrigWithRepeat(1 << 6);
}

/**
 * Checks whether the Up button was pressed this frame or repeats.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame or repeats
 */
bool isPadRepeatUp(s32 port) {
    return getController(port)->isTrigWithRepeat(1 << 16);
}

/**
 * Checks whether the Down button was pressed this frame or repeats.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame or repeats
 */
bool isPadRepeatDown(s32 port) {
    return getController(port)->isTrigWithRepeat(1 << 17);
}

/**
 * Checks whether the Left button was pressed this frame or repeats.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame or repeats
 */
bool isPadRepeatLeft(s32 port) {
    return getController(port)->isTrigWithRepeat(1 << 18);
}

/**
 * Checks whether the Right button was pressed this frame or repeats.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame or repeats
 */
bool isPadRepeatRight(s32 port) {
    return getController(port)->isTrigWithRepeat(1 << 19);
}

/**
 * Checks whether the Home button was pressed this frame or repeats.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame or repeats
 */
bool isPadRepeatHome(s32 port) {
    return getController(port)->isTrigWithRepeat(1 << 8);
}

/**
 * Checks whether the Start button was pressed this frame or repeats.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame or repeats
 */
bool isPadRepeatStart(s32 port) {
    return getController(port)->isTrigWithRepeat(1 << 11);
}

/**
 * Checks whether the Select button was pressed this frame or repeats.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame or repeats
 */
bool isPadRepeatSelect(s32 port) {
    return getController(port)->isTrigWithRepeat(1 << 12);
}

/**
 * Checks whether the Plus button was pressed this frame or repeats.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame or repeats
 */
bool isPadRepeatPlus(s32 port) {
    return getController(port)->isTrigWithRepeat(1 << 10);
}

/**
 * Checks whether the Minus button was pressed this frame or repeats.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame or repeats
 */
bool isPadRepeatMinus(s32 port) {
    return getController(port)->isTrigWithRepeat(1 << 9);
}

/**
 * Checks whether the Touch button was pressed this frame or repeats.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame or repeats
 */
bool isPadRepeatTouch(s32 port) {
    return getController(port)->isTrigWithRepeat(1 << 15);
}

/**
 * Checks whether the UpLeftStick direction was pressed this frame or repeats.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame or repeats
 */
bool isPadRepeatUpLeftStick(s32 port) {
    return getController(port)->isTrigWithRepeat(1 << 20);
}

/**
 * Checks whether the DownLeftStick direction was pressed this frame or repeats.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame or repeats
 */
bool isPadRepeatDownLeftStick(s32 port) {
    return getController(port)->isTrigWithRepeat(1 << 21);
}

/**
 * Checks whether the LeftLeftStick direction was pressed this frame or repeats.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame or repeats
 */
bool isPadRepeatLeftLeftStick(s32 port) {
    return getController(port)->isTrigWithRepeat(1 << 22);
}

/**
 * Checks whether the RightLeftStick direction was pressed this frame or repeats.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame or repeats
 */
bool isPadRepeatRightLeftStick(s32 port) {
    return getController(port)->isTrigWithRepeat(1 << 23);
}

/**
 * Checks whether the UpRightStick direction was pressed this frame or repeats.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame or repeats
 */
bool isPadRepeatUpRightStick(s32 port) {
    return getController(port)->isTrigWithRepeat(1 << 24);
}

/**
 * Checks whether the DownRightStick direction was pressed this frame or repeats.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame or repeats
 */
bool isPadRepeatDownRightStick(s32 port) {
    return getController(port)->isTrigWithRepeat(1 << 25);
}

/**
 * Checks whether the LeftRightStick direction was pressed this frame or repeats.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame or repeats
 */
bool isPadRepeatLeftRightStick(s32 port) {
    return getController(port)->isTrigWithRepeat(1 << 26);
}

/**
 * Checks whether the RightRightStick direction was pressed this frame or repeats.
 * @param port controller port, or -1 for the main controller
 * @return whether it was pressed this frame or repeats
 */
bool isPadRepeatRightRightStick(s32 port) {
    return getController(port)->isTrigWithRepeat(1 << 27);
}

/**
 * Checks whether the A button is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldA(s32 port) {
    return getController(port)->isHold(1 << 0);
}

/**
 * Checks whether the B button is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldB(s32 port) {
    return getController(port)->isHold(1 << 1);
}

/**
 * Checks whether the X button is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldX(s32 port) {
    return getController(port)->isHold(1 << 3);
}

/**
 * Checks whether the Y button is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldY(s32 port) {
    return getController(port)->isHold(1 << 4);
}

/**
 * Checks whether the ZL button is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldZL(s32 port) {
    return getController(port)->isHold(1 << 2);
}

/**
 * Checks whether the ZR button is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldZR(s32 port) {
    return getController(port)->isHold(1 << 5);
}

/**
 * Checks whether the L button is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldL(s32 port) {
    return getController(port)->isHold(1 << 13);
}

/**
 * Checks whether the R button is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldR(s32 port) {
    return getController(port)->isHold(1 << 14);
}

/**
 * Checks whether the 1 button is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHold1(s32 port) {
    return getController(port)->isHold(1 << 7);
}

/**
 * Checks whether the 2 button is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHold2(s32 port) {
    return getController(port)->isHold(1 << 6);
}

/**
 * Checks whether the Up button is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldUp(s32 port) {
    return getController(port)->isHold(1 << 16);
}

/**
 * Checks whether the Down button is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldDown(s32 port) {
    return getController(port)->isHold(1 << 17);
}

/**
 * Checks whether the Left button is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldLeft(s32 port) {
    return getController(port)->isHold(1 << 18);
}

/**
 * Checks whether the Right button is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldRight(s32 port) {
    return getController(port)->isHold(1 << 19);
}

/**
 * Checks whether both buttons of a diagonal direction are held.
 * @param port controller port, or -1 for the main controller
 * @return whether the direction is held
 */
bool isPadHoldLeftUp(s32 port) {
    return getController(port)->isHoldAll(0x50000);
}

/**
 * Checks whether both buttons of a diagonal direction are held.
 * @param port controller port, or -1 for the main controller
 * @return whether the direction is held
 */
bool isPadHoldLeftDown(s32 port) {
    return getController(port)->isHoldAll(0x60000);
}

/**
 * Checks whether both buttons of a diagonal direction are held.
 * @param port controller port, or -1 for the main controller
 * @return whether the direction is held
 */
bool isPadHoldRightUp(s32 port) {
    return getController(port)->isHoldAll(0x90000);
}

/**
 * Checks whether both buttons of a diagonal direction are held.
 * @param port controller port, or -1 for the main controller
 * @return whether the direction is held
 */
bool isPadHoldRightDown(s32 port) {
    return getController(port)->isHoldAll(0xA0000);
}

/**
 * Checks whether the Home button is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldHome(s32 port) {
    return getController(port)->isHold(1 << 8);
}

/**
 * Checks whether the Start button is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldStart(s32 port) {
    return getController(port)->isHold(1 << 11);
}

/**
 * Checks whether the Select button is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldSelect(s32 port) {
    return getController(port)->isHold(1 << 12);
}

/**
 * Checks whether the Plus button is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldPlus(s32 port) {
    return getController(port)->isHold(1 << 10);
}

/**
 * Checks whether the Minus button is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldMinus(s32 port) {
    return getController(port)->isHold(1 << 9);
}

/**
 * Checks whether any button is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldAny(s32 port) {
    return getController(port)->isHold(0xFFF7FFF);
}

/**
 * Checks whether any button except the sticks is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldAnyWithoutStick(s32 port) {
    return getController(port)->isHold(0xF7FFF);
}

/**
 * Checks whether the Touch button is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldTouch(s32 port) {
    return getController(port)->isHold(1 << 15);
}

/**
 * Checks whether the UpLeftStick direction is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldUpLeftStick(s32 port) {
    return getController(port)->isHold(1 << 20);
}

/**
 * Checks whether the DownLeftStick direction is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldDownLeftStick(s32 port) {
    return getController(port)->isHold(1 << 21);
}

/**
 * Checks whether the LeftLeftStick direction is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldLeftLeftStick(s32 port) {
    return getController(port)->isHold(1 << 22);
}

/**
 * Checks whether the RightLeftStick direction is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldRightLeftStick(s32 port) {
    return getController(port)->isHold(1 << 23);
}

/**
 * Checks whether the UpRightStick direction is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldUpRightStick(s32 port) {
    return getController(port)->isHold(1 << 24);
}

/**
 * Checks whether the DownRightStick direction is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldDownRightStick(s32 port) {
    return getController(port)->isHold(1 << 25);
}

/**
 * Checks whether the LeftRightStick direction is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldLeftRightStick(s32 port) {
    return getController(port)->isHold(1 << 26);
}

/**
 * Checks whether the RightRightStick direction is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldRightRightStick(s32 port) {
    return getController(port)->isHold(1 << 27);
}

/**
 * Checks whether the left stick direction is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldLeftStick(s32 port) {
    return getController(port)->isHold(0xF00000);
}

/**
 * Checks whether the right stick direction is held.
 * @param port controller port, or -1 for the main controller
 * @return whether it is held
 */
bool isPadHoldRightStick(s32 port) {
    return getController(port)->isHold(0xF000000);
}

/**
 * Checks whether the A button was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was released this frame
 */
bool isPadReleaseA(s32 port) {
    return getController(port)->isRelease(1 << 0);
}

/**
 * Checks whether the B button was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was released this frame
 */
bool isPadReleaseB(s32 port) {
    return getController(port)->isRelease(1 << 1);
}

/**
 * Checks whether the X button was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was released this frame
 */
bool isPadReleaseX(s32 port) {
    return getController(port)->isRelease(1 << 3);
}

/**
 * Checks whether the Y button was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was released this frame
 */
bool isPadReleaseY(s32 port) {
    return getController(port)->isRelease(1 << 4);
}

/**
 * Checks whether the ZL button was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was released this frame
 */
bool isPadReleaseZL(s32 port) {
    return getController(port)->isRelease(1 << 2);
}

/**
 * Checks whether the ZR button was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was released this frame
 */
bool isPadReleaseZR(s32 port) {
    return getController(port)->isRelease(1 << 5);
}

/**
 * Checks whether the L button was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was released this frame
 */
bool isPadReleaseL(s32 port) {
    return getController(port)->isRelease(1 << 13);
}

/**
 * Checks whether the R button was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was released this frame
 */
bool isPadReleaseR(s32 port) {
    return getController(port)->isRelease(1 << 14);
}

/**
 * Checks whether the 1 button was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was released this frame
 */
bool isPadRelease1(s32 port) {
    return getController(port)->isRelease(1 << 7);
}

/**
 * Checks whether the 2 button was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was released this frame
 */
bool isPadRelease2(s32 port) {
    return getController(port)->isRelease(1 << 6);
}

/**
 * Checks whether the Up button was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was released this frame
 */
bool isPadReleaseUp(s32 port) {
    return getController(port)->isRelease(1 << 16);
}

/**
 * Checks whether the Down button was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was released this frame
 */
bool isPadReleaseDown(s32 port) {
    return getController(port)->isRelease(1 << 17);
}

/**
 * Checks whether the Left button was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was released this frame
 */
bool isPadReleaseLeft(s32 port) {
    return getController(port)->isRelease(1 << 18);
}

/**
 * Checks whether the Right button was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was released this frame
 */
bool isPadReleaseRight(s32 port) {
    return getController(port)->isRelease(1 << 19);
}

/**
 * Checks whether the Home button was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was released this frame
 */
bool isPadReleaseHome(s32 port) {
    return getController(port)->isRelease(1 << 8);
}

/**
 * Checks whether the Start button was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was released this frame
 */
bool isPadReleaseStart(s32 port) {
    return getController(port)->isRelease(1 << 11);
}

/**
 * Checks whether the Select button was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was released this frame
 */
bool isPadReleaseSelect(s32 port) {
    return getController(port)->isRelease(1 << 12);
}

/**
 * Checks whether the Plus button was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was released this frame
 */
bool isPadReleasePlus(s32 port) {
    return getController(port)->isRelease(1 << 10);
}

/**
 * Checks whether the Minus button was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was released this frame
 */
bool isPadReleaseMinus(s32 port) {
    return getController(port)->isRelease(1 << 9);
}

/**
 * Checks whether the Touch button was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was released this frame
 */
bool isPadReleaseTouch(s32 port) {
    return getController(port)->isRelease(1 << 15);
}

/**
 * Checks whether the UpLeftStick direction was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was released this frame
 */
bool isPadReleaseUpLeftStick(s32 port) {
    return getController(port)->isRelease(1 << 20);
}

/**
 * Checks whether the DownLeftStick direction was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was released this frame
 */
bool isPadReleaseDownLeftStick(s32 port) {
    return getController(port)->isRelease(1 << 21);
}

/**
 * Checks whether the LeftLeftStick direction was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was released this frame
 */
bool isPadReleaseLeftLeftStick(s32 port) {
    return getController(port)->isRelease(1 << 22);
}

/**
 * Checks whether the RightLeftStick direction was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was released this frame
 */
bool isPadReleaseRightLeftStick(s32 port) {
    return getController(port)->isRelease(1 << 23);
}

/**
 * Checks whether the UpRightStick direction was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was released this frame
 */
bool isPadReleaseUpRightStick(s32 port) {
    return getController(port)->isRelease(1 << 24);
}

/**
 * Checks whether the DownRightStick direction was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was released this frame
 */
bool isPadReleaseDownRightStick(s32 port) {
    return getController(port)->isRelease(1 << 25);
}

/**
 * Checks whether the LeftRightStick direction was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was released this frame
 */
bool isPadReleaseLeftRightStick(s32 port) {
    return getController(port)->isRelease(1 << 26);
}

/**
 * Checks whether the RightRightStick direction was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether it was released this frame
 */
bool isPadReleaseRightRightStick(s32 port) {
    return getController(port)->isRelease(1 << 27);
}

/**
 * Checks whether the A, B, X or Y button was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether one of the buttons was pressed
 */
bool isPadTriggerAnyABXY(s32 port) {
    return isPadTriggerA(port) || isPadTriggerB(port) || isPadTriggerX(port) ||
           isPadTriggerY(port);
}

/**
 * Checks whether the A, B, X or Y button is held.
 * @param port controller port, or -1 for the main controller
 * @return whether one of the buttons is held
 */
bool isPadHoldAnyABXY(s32 port) {
    return getController(port)->isHold(0x1b);
}

/**
 * Returns the direction held on the directional buttons.
 * @param pDir output direction
 * @param port controller port, or -1 for the main controller
 */
void getPadCrossDir(sead::Vector2f* pDir, s32 port) {
    pDir->x = 0.0f;
    pDir->y = 0.0f;

    if (isPadHoldUp(port)) {
        pDir->y = 1.0f;
    }

    if (isPadHoldDown(port)) {
        pDir->y = -1.0f;
    }

    if (isPadHoldLeft(port)) {
        pDir->x = -1.0f;
    }

    if (isPadHoldRight(port)) {
        pDir->x = 1.0f;
    }
}

/**
 * Returns the direction held on the directional buttons of a sideways Joy-Con.
 * @param pDir output direction
 * @param port controller port, or -1 for the main controller
 */
void getPadCrossDirSideways(sead::Vector2f* pDir, s32 port) {
    pDir->x = 0.0f;
    pDir->y = 0.0f;

    if (isPadHoldUp(port)) {
        pDir->x = -1.0f;
    }

    if (isPadHoldDown(port)) {
        pDir->x = 1.0f;
    }

    if (isPadHoldLeft(port)) {
        pDir->y = -1.0f;
    }

    if (isPadHoldRight(port)) {
        pDir->y = 1.0f;
    }
}

/**
 * Calculates the touch position in sub display coordinates.
 * @param pPos touch position
 * @param port controller port, or -1 for the main controller
 */
void calcTouchScreenPos(sead::Vector2f* pPos, s32 port) {
    pPos->set(getController(port)->getPointer());
    pPos->x = pPos->x * getSubDisplayWidth() / 1280.0f;
    pPos->y = pPos->y * getSubDisplayHeight() / 720.0f;
}

/**
 * Calculates the touch position in layout coordinates.
 * @param pPos touch position
 * @param port controller port, or -1 for the main controller
 */
void calcTouchLayoutPos(sead::Vector2f* pPos, s32 port) {
    const sead::Vector2f& pointer = getController(port)->getPointer();
    f32 x = pointer.x;
    f32 y = pointer.y;
    f32 width = getLayoutDisplayWidth();
    f32 height = getLayoutDisplayHeight();
    pPos->set(x - width * 0.5f, -(y - height * 0.5f));
}

/**
 * Checks whether the touch position is inside a rectangle.
 * @param rRectPos top left corner of the rectangle
 * @param rSize size of the rectangle
 * @param port controller port, or -1 for the main controller
 * @return whether the touch position is inside
 */
bool isTouchPosInRect(const sead::Vector2f& rRectPos, const sead::Vector2f& rSize, s32 port) {
    sead::Vector2f pos;
    calcTouchScreenPos(&pos, port);
    return rRectPos.x <= pos.x && pos.x < rRectPos.x + rSize.x && rRectPos.y <= pos.y &&
           pos.y < rRectPos.y + rSize.y;
}

/**
 * Checks whether the touch position is inside a circle.
 * @param rCenter center of the circle
 * @param radius radius of the circle
 * @param port controller port, or -1 for the main controller
 * @return whether the touch position is inside
 */
bool isTouchPosInCircle(const sead::Vector2f& rCenter, f32 radius, s32 port) {
    sead::Vector2f pos;
    calcTouchScreenPos(&pos, port);
    return (pos - rCenter).squaredLength() <= radius * radius;
}

/**
 * Checks whether a world position is touched.
 * @param rPos world position
 * @param pCamera camera user
 * @param radius touch radius
 * @param depth depth of the touch
 * @param port controller port
 * @return always false
 */
bool isTouchPosInCircleByWorldPos(const sead::Vector3f& rPos, const IUseCamera* pCamera,
                                  f32 radius, f32 depth, s32 port) {
    return false;
}

/**
 * Checks whether the touch panel is touched inside a rectangle.
 * @param left left edge of the rectangle
 * @param top top edge of the rectangle
 * @param width width of the rectangle
 * @param height height of the rectangle
 * @param port controller port, or -1 for the main controller
 * @return whether the touch position is inside
 */
bool isPadTouchRect(f32 left, f32 top, f32 width, f32 height, s32 port) {
    if (!isPadHoldTouch(port)) {
        return false;
    }

    sead::Vector2f pos;
    calcTouchScreenPos(&pos, port);
    return pos.x >= left && pos.x < left + width && pos.y >= top && pos.y < top + height;
}

/**
 * Checks whether the UI cursor up direction was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether the direction was pressed this frame
 */
bool isPadTriggerUiCursorUp(s32 port) {
    return getUiKeyInputAddon(port)->getPadTrig().isOnBit(0);
}

/**
 * Checks whether the UI cursor down direction was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether the direction was pressed this frame
 */
bool isPadTriggerUiCursorDown(s32 port) {
    return getUiKeyInputAddon(port)->getPadTrig().isOnBit(1);
}

/**
 * Checks whether the UI cursor left direction was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether the direction was pressed this frame
 */
bool isPadTriggerUiCursorLeft(s32 port) {
    return getUiKeyInputAddon(port)->getPadTrig().isOnBit(2);
}

/**
 * Checks whether the UI cursor right direction was pressed this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether the direction was pressed this frame
 */
bool isPadTriggerUiCursorRight(s32 port) {
    return getUiKeyInputAddon(port)->getPadTrig().isOnBit(3);
}

/**
 * Checks whether the UI cursor up direction was pressed this frame or is repeating.
 * @param port controller port, or -1 for the main controller
 * @return whether the direction was pressed this frame or is repeating
 */
bool isPadRepeatUiCursorUp(s32 port) {
    return getUiKeyInputAddon(port)->getPadRepeat().isOnBit(0);
}

/**
 * Checks whether the UI cursor down direction was pressed this frame or is repeating.
 * @param port controller port, or -1 for the main controller
 * @return whether the direction was pressed this frame or is repeating
 */
bool isPadRepeatUiCursorDown(s32 port) {
    return getUiKeyInputAddon(port)->getPadRepeat().isOnBit(1);
}

/**
 * Checks whether the UI cursor left direction was pressed this frame or is repeating.
 * @param port controller port, or -1 for the main controller
 * @return whether the direction was pressed this frame or is repeating
 */
bool isPadRepeatUiCursorLeft(s32 port) {
    return getUiKeyInputAddon(port)->getPadRepeat().isOnBit(2);
}

/**
 * Checks whether the UI cursor right direction was pressed this frame or is repeating.
 * @param port controller port, or -1 for the main controller
 * @return whether the direction was pressed this frame or is repeating
 */
bool isPadRepeatUiCursorRight(s32 port) {
    return getUiKeyInputAddon(port)->getPadRepeat().isOnBit(3);
}

/**
 * Checks whether the UI cursor up direction is held.
 * @param port controller port, or -1 for the main controller
 * @return whether the direction is held
 */
bool isPadHoldUiCursorUp(s32 port) {
    return getUiKeyInputAddon(port)->getPadHold().isOnBit(0);
}

/**
 * Checks whether the UI cursor down direction is held.
 * @param port controller port, or -1 for the main controller
 * @return whether the direction is held
 */
bool isPadHoldUiCursorDown(s32 port) {
    return getUiKeyInputAddon(port)->getPadHold().isOnBit(1);
}

/**
 * Checks whether the UI cursor left direction is held.
 * @param port controller port, or -1 for the main controller
 * @return whether the direction is held
 */
bool isPadHoldUiCursorLeft(s32 port) {
    return getUiKeyInputAddon(port)->getPadHold().isOnBit(2);
}

/**
 * Checks whether the UI cursor right direction is held.
 * @param port controller port, or -1 for the main controller
 * @return whether the direction is held
 */
bool isPadHoldUiCursorRight(s32 port) {
    return getUiKeyInputAddon(port)->getPadHold().isOnBit(3);
}

/**
 * Checks whether the UI cursor up direction was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether the direction was released this frame
 */
bool isPadReleaseUiCursorUp(s32 port) {
    return getUiKeyInputAddon(port)->getPadHoldAndPrev().isOnBit(0);
}

/**
 * Checks whether the UI cursor down direction was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether the direction was released this frame
 */
bool isPadReleaseUiCursorDown(s32 port) {
    return getUiKeyInputAddon(port)->getPadHoldAndPrev().isOnBit(1);
}

/**
 * Checks whether the UI cursor left direction was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether the direction was released this frame
 */
bool isPadReleaseUiCursorLeft(s32 port) {
    return getUiKeyInputAddon(port)->getPadHoldAndPrev().isOnBit(2);
}

/**
 * Checks whether the UI cursor right direction was released this frame.
 * @param port controller port, or -1 for the main controller
 * @return whether the direction was released this frame
 */
bool isPadReleaseUiCursorRight(s32 port) {
    return getUiKeyInputAddon(port)->getPadHoldAndPrev().isOnBit(3);
}

/**
 * Returns the number of accelerometers of a controller.
 * @param port controller port, or -1 for the main controller
 * @return number of accelerometers
 */
s32 getPadAccelerationDeviceNum(s32 port) {
    if (port >= 0) {
        if (NpadController* npad = tryGetNpadController(port)) {
            return npad->getSixAxisSensorNum();
        }
    } else if (port == -1) {
        port = getMainControllerPort();
    }

    sead::Controller* controller = sead::ControllerMgr::instance()->getController(port);
    return (controller->getAddonByOrderAs<sead::AccelerometerAddon*>(0) != nullptr) ? 1 : 0;
}

/**
 * Gets the acceleration of a controller.
 * @param pAcceleration acceleration
 * @param port controller port
 * @param index accelerometer index
 * @return whether the acceleration is valid
 */
bool tryGetPadAcceleration(sead::Vector3f* pAcceleration, s32 port, s32 index) {
    sead::Controller* controller = sead::ControllerMgr::instance()->getController(port);

    if (controller == nullptr || !controller->isConnected()) {
        pAcceleration->set(0.0f, 0.0f, 0.0f);
        return false;
    }

    controller = sead::ControllerMgr::instance()->getController(port);
    auto* addon = controller->getAddonByOrderAs<sead::AccelerometerAddon*>(index);

    if (addon == nullptr) {
        return false;
    }

    pAcceleration->set(addon->getAcceleration());
    return addon->isEnable();
}

/**
 * Gets the acceleration of a controller and the time since the last sample.
 * @param pAcceleration acceleration
 * @param pDeltaTime time since the last sample
 * @param port controller port
 * @param index accelerometer index
 * @return whether the acceleration is valid
 */
bool tryGetPadAccerationAndTimeDelta(sead::Vector3f* pAcceleration, nn::TimeSpanType* pDeltaTime,
                                     s32 port, s32 index) {
    sead::Controller* controller = sead::ControllerMgr::instance()->getController(port);

    if (controller == nullptr || !controller->isConnected()) {
        pAcceleration->set(0.0f, 0.0f, 0.0f);
        return false;
    }

    controller = sead::ControllerMgr::instance()->getController(port);
    auto* addon = controller->getAddonByOrderAs<JoyPadAccelerometerAddon*>(index);

    if (addon == nullptr) {
        return false;
    }

    pAcceleration->set(addon->getAcceleration());
    pDeltaTime->_nanoSeconds = addon->getDeltaTime();
    return addon->isEnable();
}

/**
 * Checks whether a controller is being shaken.
 * @param threshold minimum difference of the acceleration from gravity
 * @param port controller port
 * @param index accelerometer index
 * @return whether the controller is being shaken
 */
bool isShakePadAcceleration(f32 threshold, s32 port, s32 index) {
    sead::Vector3f acceleration = {0.0f, 0.0f, 0.0f};

    if (!tryGetPadAcceleration(&acceleration, port, index)) {
        return false;
    }

    return sead::Mathf::abs(1.0f - acceleration.length()) > threshold;
}

/**
 * Checks whether a controller has a usable right stick.
 * @param port controller port, or -1 for the main controller
 * @return whether it is not a single Joy-Con
 */
bool isPadEnableRightStick(s32 port) {
    if (port == -1) {
        port = getMainControllerPort();
    }

    if (isPadStyle(port, sead::NinJoyNpadDevice::cStyle_JoyLeft)) {
        return false;
    }

    if (isPadStyle(port, sead::NinJoyNpadDevice::cStyle_JoyRight)) {
        return false;
    }

    return true;
}

/**
 * Checks whether a controller is a single Joy-Con.
 * @param port controller port, or -1 for the main controller
 * @return whether it is a single Joy-Con
 */
bool isPadTypeJoySingle(s32 port) {
    if (port == -1) {
        port = getMainControllerPort();
    }

    return isPadStyle(port, sead::NinJoyNpadDevice::cStyle_JoyLeft) ||
           isPadStyle(port, sead::NinJoyNpadDevice::cStyle_JoyRight);
}

/**
 * Returns the number of frames a controller has been idle.
 * @param port controller port, or -1 for the main controller
 * @return idle frames
 */
s32 getPadIdleFrame(s32 port) {
    return getController(port)->getIdleFrame();
}

/**
 * Checks whether a controller waits for a connection.
 * @param port controller port
 * @return whether it waits for a connection
 */
bool isPadWaitingConnect(s32 port) {
    auto* npad =
        sead::DynamicCast<NpadController>(sead::ControllerMgr::instance()->getController(port));
    if (npad == nullptr) {
        return false;
    }

    return npad->isWaitingConnect();
}

/**
 * Checks whether every connected Npad is a single Joy-Con.
 * @return whether only single Joy-Cons are connected
 */
bool isSingleJoyConOnly() {
    for (u32 i = 0; i < 4; i++) {
        sead::Controller* controller = sead::ControllerMgr::instance()->getController(i);

        if (controller == nullptr || !controller->isConnected()) {
            continue;
        }

        if (tryGetNpadController(i) == nullptr) {
            continue;
        }

        if (!isPadStyle(i, sead::NinJoyNpadDevice::cStyle_JoyLeft) &&
            !isPadStyle(i, sead::NinJoyNpadDevice::cStyle_JoyRight)) {
            return false;
        }
    }

    return true;
}

/**
 * Checks whether a controller is connected.
 * @param port controller port
 * @return whether it is connected
 */
bool isPadConnected(s32 port) {
    sead::Controller* controller = sead::ControllerMgr::instance()->getController(port);

    if (controller == nullptr) {
        return false;
    }

    return controller->isConnected();
}

/**
 * Checks whether a controller has a usable left stick.
 * @param port controller port
 * @return always false
 */
bool isPadEnableLeftStick(s32 port) {
    return false;
}

/**
 * Checks whether a controller has a speaker.
 * @param port controller port
 * @return always false
 */
bool isPadExistDeviceSpeaker(s32 port) {
    return false;
}

/**
 * Returns the first connected port of a list.
 * @param pPorts ports to check
 * @param portNum number of ports
 * @return the first connected port, or -1
 */
s32 tryGetPortFirstConnected(const s32* pPorts, s32 portNum) {
    for (s32 i = 0; i < portNum; i++) {
        if (isPadConnected(pPorts[i])) {
            return pPorts[i];
        }
    }

    return -1;
}

/**
 * Checks whether the main Npad is connected.
 * @return whether it is connected
 */
bool isPadConnectedJoyPadDouble() {
    return isPadConnected(getJoyPadDoublePort());
}

/**
 * Returns the number of gyro sensors of a controller.
 * @param port controller port, or -1 for the main controller
 * @return number of gyro sensors
 */
s32 getPadPoseDeviceNum(s32 port) {
    if (port >= 0) {
        if (NpadController* npad = tryGetNpadController(port)) {
            return npad->getSixAxisSensorNum();
        }
    } else if (port == -1) {
        port = getMainControllerPort();
    }

    sead::Controller* controller = sead::ControllerMgr::instance()->getController(port);
    return (controller->getAddonByOrderAs<PadGyroAddon*>(0) != nullptr) ? 1 : 0;
}

/**
 * Gets the pose of a controller.
 * @param pSide side vector, may be nullptr
 * @param pUp up vector, may be nullptr
 * @param pFront front vector, may be nullptr
 * @param port controller port, or -1 for the main controller
 * @param index gyro index
 * @return whether a valid pose was read
 */
bool tryGetPadPose(sead::Vector3f* pSide, sead::Vector3f* pUp, sead::Vector3f* pFront, s32 port,
                   s32 index) {
    return getPadPose(pSide, pUp, pFront, port, index);
}

/**
 * Gets the pose of a controller.
 * @param pSide side vector, may be nullptr
 * @param pUp up vector, may be nullptr
 * @param pFront front vector, may be nullptr
 * @param port controller port, or -1 for the main controller
 * @param index gyro index
 * @return whether a valid pose was read
 */
bool getPadPose(sead::Vector3f* pSide, sead::Vector3f* pUp, sead::Vector3f* pFront, s32 port,
                s32 index) {
    PadGyroAddon* addon = tryGetGyroAddon(port, index);

    if (addon != nullptr && addon->isStatusOk()) {
        addon->getPose(pSide, pUp, pFront);
        bool isValid = true;

        if (pSide && isNearZero(*pSide)) {
            pSide->set(sead::Vector3f::ex);
            isValid = false;
        }

        if (pUp && isNearZero(*pUp)) {
            pUp->set(sead::Vector3f::ey);
            isValid = false;
        }

        if (pFront && isNearZero(*pFront)) {
            pFront->set(sead::Vector3f::ez);
            isValid = false;
        }

        return isValid;
    }

    if (pSide) {
        pSide->set(sead::Vector3f::ex);
    }

    if (pUp) {
        pUp->set(sead::Vector3f::ey);
    }

    if (pFront) {
        pFront->set(sead::Vector3f::ez);
    }

    return false;
}

/**
 * Gets the pose reported by the SDK of a controller.
 * @param pSide side vector, may be nullptr
 * @param pUp up vector, may be nullptr
 * @param pFront front vector, may be nullptr
 * @param port controller port, or -1 for the main controller
 * @param index gyro index
 * @return whether a valid pose was read
 */
bool getPadSDKPose(sead::Vector3f* pSide, sead::Vector3f* pUp, sead::Vector3f* pFront, s32 port,
                   s32 index) {
    PadGyroAddon* addon = tryGetGyroAddon(port, index);

    if (addon != nullptr && addon->isStatusOk()) {
        addon->getSDKPose(pSide, pUp, pFront);
        bool isValid = true;

        if (pSide && isNearZero(*pSide)) {
            pSide->set(sead::Vector3f::ex);
            isValid = false;
        }

        if (pUp && isNearZero(*pUp)) {
            pUp->set(sead::Vector3f::ey);
            isValid = false;
        }

        if (pFront && isNearZero(*pFront)) {
            pFront->set(sead::Vector3f::ez);
            isValid = false;
        }

        return isValid;
    }

    if (pSide) {
        pSide->set(sead::Vector3f::ex);
    }

    if (pUp) {
        pUp->set(sead::Vector3f::ey);
    }

    if (pFront) {
        pFront->set(sead::Vector3f::ez);
    }

    return false;
}

/**
 * Gets the pose of a controller as a 3x3 matrix.
 * @param pMtx pose matrix
 * @param port controller port, or -1 for the main controller
 * @param index gyro index
 */
void getPadPoseMtx(sead::Matrix33f* pMtx, s32 port, s32 index) {
    sead::Vector3f side = sead::Vector3f::ex;
    sead::Vector3f up = sead::Vector3f::ey;
    sead::Vector3f front = sead::Vector3f::ez;
    getPadPose(&side, &up, &front, port, index);
    pMtx->setBase(0, side);
    pMtx->setBase(1, up);
    pMtx->setBase(2, front);
}

/**
 * Gets the pose of a controller as a 3x4 matrix.
 * @param pMtx pose matrix
 * @param port controller port, or -1 for the main controller
 * @param index gyro index
 */
void getPadPoseMtx(sead::Matrix34f* pMtx, s32 port, s32 index) {
    pMtx->makeIdentity();
    sead::Vector3f side = sead::Vector3f::ex;
    sead::Vector3f up = sead::Vector3f::ey;
    sead::Vector3f front = sead::Vector3f::ez;
    getPadPose(&side, &up, &front, port, index);
    pMtx->setBase(0, side);
    pMtx->setBase(1, up);
    pMtx->setBase(2, front);
}

/**
 * Gets the pose of a controller as a 3x3 matrix if it is valid.
 * @param pMtx pose matrix
 * @param port controller port, or -1 for the main controller
 * @param index gyro index
 * @return whether a valid pose was read
 */
bool tryGetPadPoseMtx(sead::Matrix33f* pMtx, s32 port, s32 index) {
    sead::Vector3f side = sead::Vector3f::ex;
    sead::Vector3f up = sead::Vector3f::ey;
    sead::Vector3f front = sead::Vector3f::ez;

    if (!getPadPose(&side, &up, &front, port, index)) {
        return false;
    }

    pMtx->setBase(0, side);
    pMtx->setBase(1, up);
    pMtx->setBase(2, front);
    return true;
}

/**
 * Gets the pose of a controller as a 3x4 matrix if it is valid.
 * @param pMtx pose matrix
 * @param port controller port, or -1 for the main controller
 * @param index gyro index
 * @return whether a valid pose was read
 */
bool tryGetPadPoseMtx(sead::Matrix34f* pMtx, s32 port, s32 index) {
    pMtx->makeIdentity();
    sead::Vector3f side = sead::Vector3f::ex;
    sead::Vector3f up = sead::Vector3f::ey;
    sead::Vector3f front = sead::Vector3f::ez;

    if (!getPadPose(&side, &up, &front, port, index)) {
        return false;
    }

    pMtx->setBase(0, side);
    pMtx->setBase(1, up);
    pMtx->setBase(2, front);
    return true;
}

/**
 * Gets the rotation angles of a controller.
 * @param pAngle angles in radians
 * @param port controller port, or -1 for the main controller
 * @param index gyro index
 */
void getPadPoseRadian(sead::Vector3f* pAngle, s32 port, s32 index) {
    PadGyroAddon* addon = tryGetGyroAddon(port, index);

    if (addon != nullptr && addon->isStatusOk()) {
        pAngle->set(addon->getAngle());
        *pAngle *= sead::Mathf::pi2();
    }
}

/**
 * Gets the pose reported by the SDK of a controller as a matrix.
 * @param pMtx pose matrix
 * @param port controller port, or -1 for the main controller
 * @param index gyro index
 */
void getPadSDKPoseMtx(sead::Matrix34f* pMtx, s32 port, s32 index) {
    pMtx->makeIdentity();
    sead::Vector3f side = sead::Vector3f::ex;
    sead::Vector3f up = sead::Vector3f::ey;
    sead::Vector3f front = sead::Vector3f::ez;
    getPadSDKPose(&side, &up, &front, port, index);
    pMtx->setBase(0, side);
    pMtx->setBase(1, up);
    pMtx->setBase(2, front);
}

/**
 * Gets the angular velocity of a controller.
 * @param pVelocity angular velocity
 * @param port controller port, or -1 for the main controller
 * @param index gyro index
 */
void getPadAngularVelocity(sead::Vector3f* pVelocity, s32 port, s32 index) {
    PadGyroAddon* addon = tryGetGyroAddon(port, index);

    if (addon != nullptr && addon->isStatusOk()) {
        pVelocity->set(addon->getAngularVelocity());
    }
}

/**
 * Gets the pose of the main left single Joy-Con.
 * @param pSide side vector, may be nullptr
 * @param pUp up vector, may be nullptr
 * @param pFront front vector, may be nullptr
 */
void getPadPoseMainJoySingleLeft(sead::Vector3f* pSide, sead::Vector3f* pUp,
                                 sead::Vector3f* pFront) {
    getPadPose(pSide, pUp, pFront, getMainJoyPadSingleLeftPort(), 0);
}

/**
 * Gets the pose of the main right single Joy-Con.
 * @param pSide side vector, may be nullptr
 * @param pUp up vector, may be nullptr
 * @param pFront front vector, may be nullptr
 */
void getPadPoseMainJoySingleRight(sead::Vector3f* pSide, sead::Vector3f* pUp,
                                  sead::Vector3f* pFront) {
    getPadPose(pSide, pUp, pFront, getMainJoyPadSingleRightPort(), 1);
}

/**
 * Resets the pose of a controller.
 * @param port controller port
 */
void resetPadPose(s32 port) {}

/**
 * Resets the auto sleep timer of a controller.
 * @param port controller port
 */
void resetPadAutoSleepTimeCount(s32 port) {}

/**
 * Checks whether a controller is a Joy-Con pair.
 * @param port controller port, or -1 for the main controller
 * @return whether it is a Joy-Con pair
 */
bool isPadTypeJoyDual(s32 port) {
    if (port == -1) {
        port = getMainControllerPort();
    }

    return isPadStyle(port, sead::NinJoyNpadDevice::cStyle_JoyDual);
}

/**
 * Checks whether a controller is a left Joy-Con.
 * @param port controller port, or -1 for the main controller
 * @return whether it is a left Joy-Con
 */
bool isPadTypeJoyLeft(s32 port) {
    if (port == -1) {
        port = getMainControllerPort();
    }

    return isPadStyle(port, sead::NinJoyNpadDevice::cStyle_JoyLeft);
}

/**
 * Checks whether a controller is a right Joy-Con.
 * @param port controller port, or -1 for the main controller
 * @return whether it is a right Joy-Con
 */
bool isPadTypeJoyRight(s32 port) {
    if (port == -1) {
        port = getMainControllerPort();
    }

    return isPadStyle(port, sead::NinJoyNpadDevice::cStyle_JoyRight);
}

/**
 * Checks whether a controller is a handheld Joy-Con pair.
 * @param port controller port, or -1 for the main controller
 * @return whether it is a handheld Joy-Con pair
 */
bool isPadTypeHandheld(s32 port) {
    if (port == -1) {
        port = getMainControllerPort();
    }

    return isPadStyle(port, sead::NinJoyNpadDevice::cStyle_Handheld);
}

/**
 * Checks whether a controller is a Pro Controller.
 * @param port controller port, or -1 for the main controller
 * @return whether it is a Pro Controller
 */
bool isPadTypeFullKey(s32 port) {
    if (port == -1) {
        port = getMainControllerPort();
    }

    return isPadStyle(port, sead::NinJoyNpadDevice::cStyle_FullKey);
}

/**
 * Gets the colors of a Joy-Con.
 * @param pMainLeft main color of the left Joy-Con
 * @param pSubLeft sub color of the left Joy-Con
 * @param pMainRight main color of the right Joy-Con
 * @param pSubRight sub color of the right Joy-Con
 * @param port controller port, or -1 for the main controller
 * @return whether the colors could be read
 */
bool tryGetPadColor(sead::Color4f* pMainLeft, sead::Color4f* pSubLeft, sead::Color4f* pMainRight,
                    sead::Color4f* pSubRight, s32 port) {
    if (!isPadTypeJoyDual(port) && !isPadTypeJoyLeft(port) && !isPadTypeJoyRight(port)) {
        return false;
    }

    if (port == -1) {
        port = getMainControllerPort();
    }

    if (port < 0) {
        return false;
    }

    NpadController* npad = tryGetNpadController(port);

    if (npad == nullptr || !npad->isValidNpadId()) {
        return false;
    }

    nn::hid::NpadControllerColor colorLeft;
    nn::hid::NpadControllerColor colorRight;
    u32 npadId = npad->getNpadId();

    if (nn::hid::GetNpadControllerColor(&colorLeft, &colorRight, npadId).IsFailure()) {
        return false;
    }

    *pMainLeft = sead::Color4f(colorLeft.mMain.v[0] / 255.0f, colorLeft.mMain.v[1] / 255.0f,
                               colorLeft.mMain.v[2] / 255.0f, 1.0f);
    *pMainRight = sead::Color4f(colorRight.mMain.v[0] / 255.0f, colorRight.mMain.v[1] / 255.0f,
                                colorRight.mMain.v[2] / 255.0f, 1.0f);
    *pSubLeft = sead::Color4f(colorLeft.mSub.v[0] / 255.0f, colorLeft.mSub.v[1] / 255.0f,
                              colorLeft.mSub.v[2] / 255.0f, 1.0f);
    *pSubRight = sead::Color4f(colorRight.mSub.v[0] / 255.0f, colorRight.mSub.v[1] / 255.0f,
                               colorRight.mSub.v[2] / 255.0f, 1.0f);
    return true;
}

/**
 * Disconnects a controller.
 * @param port controller port
 */
void setPadDisconnect(s32 port) {}

/**
 * Checks whether the A button is held on one of the first three controllers.
 * @return whether it is held
 */
bool isEitherPadHoldA() {
    sead::ControllerMgr* mgr = sead::ControllerMgr::instance();

    if (mgr->getControllerUnsafe(0)->isHold(1 << 0) ||
        mgr->getControllerUnsafe(1)->isHold(1 << 0) ||
        mgr->getControllerUnsafe(2)->isHold(1 << 0)) {
        return true;
    }

    return false;
}

/**
 * Checks whether the B button is held on one of the first three controllers.
 * @return whether it is held
 */
bool isEitherPadHoldB() {
    sead::ControllerMgr* mgr = sead::ControllerMgr::instance();

    if (mgr->getControllerUnsafe(0)->isHold(1 << 1) ||
        mgr->getControllerUnsafe(1)->isHold(1 << 1) ||
        mgr->getControllerUnsafe(2)->isHold(1 << 1)) {
        return true;
    }

    return false;
}

/**
 * Checks whether buttons were pressed this frame on one of the first three controllers.
 * @param mask button mask
 * @return whether one of the buttons was pressed
 */
bool isEitherPadTrigger(sead::Controller::PadMask mask) {
    sead::ControllerMgr* mgr = sead::ControllerMgr::instance();

    if (mgr->getControllerUnsafe(0)->isTrig(mask) ||
        mgr->getControllerUnsafe(1)->isTrig(mask) ||
        mgr->getControllerUnsafe(2)->isTrig(mask)) {
        return true;
    }

    return false;
}
}  // namespace al

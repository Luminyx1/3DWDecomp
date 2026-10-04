#include "Util/InputUtil.hpp"

#include <controller/seadController.h>
#include <controller/seadControllerMgr.h>

#include "Library/Controller/GamePadSystem.hpp"
#include "Library/Controller/InputFunction.hpp"
#include "Library/Controller/NpadController.hpp"
#include "Library/Screen/ScreenFunction.hpp"
#include "System/Application.hpp"
#include "System/GameDataConst.hpp"
#include "System/GameDataHolderAccessor.hpp"
#include "System/GameDataHolderWriter.hpp"
#include "System/GameSystem.hpp"
#include "System/RootTask.hpp"
#include "Util/ControlUserUtil.hpp"

namespace {

/// Number of controller ports scanned by the camera input lookup.
constexpr s32 cCameraInputPortNum = 9;

/**
 * @brief Fetch the game pad system owned by the running game system.
 * @return The game pad system.
 */
inline al::GamePadSystem* getGamePadSystem() {
    return Application::instance()->getRootTask()->getGameSystem()->getGamePadSystem();
}

/**
 * @brief Fetch a controller without any port validation or replay redirection.
 * @param port Controller port.
 * @return The controller registered at the port.
 */
inline sead::Controller* getRawController(s32 port) {
    return sead::ControllerMgr::instance()->getControllerUnsafe(port);
}

}  // namespace

namespace rc {
    /**
     * @brief Checks whether the first active control user triggered the UI decide button.
     * @param accessor Accessor used to find the first active control user.
     * @return Whether it was triggered this frame.
     */
    bool isPadTriggerUiDecide(GameDataHolderAccessor accessor) {
        return isPadTriggerUiDecideByPort(calcPadPortByFirstActiveUser(accessor));
    }

    /**
     * @brief Checks whether the controller triggered the UI decide button.
     * @param port Controller port; replaced by the first connected port if disconnected.
     * @return Whether it was triggered this frame.
     */
    bool isPadTriggerUiDecideByPort(s32 port) {
        return al::isPadTriggerA(tryGetConnectCheckedPort(port));
    }

    /**
     * @brief Checks whether the first active control user triggered the UI cancel button.
     * @param accessor Accessor used to find the first active control user.
     * @return Whether it was triggered this frame.
     */
    bool isPadTriggerUiCancel(GameDataHolderAccessor accessor) {
        return isPadTriggerUiCancelByPort(calcPadPortByFirstActiveUser(accessor));
    }

    /**
     * @brief Checks whether the controller triggered the UI cancel button.
     * @param port Controller port; replaced by the first connected port if disconnected.
     * @return Whether it was triggered this frame.
     */
    bool isPadTriggerUiCancelByPort(s32 port) {
        return al::isPadTriggerB(tryGetConnectCheckedPort(port));
    }

    /**
     * @brief Checks whether the first active control user triggered UI left.
     * @param accessor Accessor used to find the first active control user.
     * @return Whether it was triggered this frame.
     */
    bool isPadTriggerUiLeft(GameDataHolderAccessor accessor) {
        return isPadTriggerUiLeftByPort(calcPadPortByFirstActiveUser(accessor));
    }

    /**
     * @brief Checks whether the controller triggered UI left.
     * @param port Controller port; replaced by the first connected port if disconnected.
     * @return Whether it was triggered this frame.
     */
    bool isPadTriggerUiLeftByPort(s32 port) {
        return isPadTriggerLeftOrStick(tryGetConnectCheckedPort(port));
    }

    /**
     * @brief Checks whether the first active control user triggered UI right.
     * @param accessor Accessor used to find the first active control user.
     * @return Whether it was triggered this frame.
     */
    bool isPadTriggerUiRight(GameDataHolderAccessor accessor) {
        return isPadTriggerUiRightByPort(calcPadPortByFirstActiveUser(accessor));
    }

    /**
     * @brief Checks whether the controller triggered UI right.
     * @param port Controller port; replaced by the first connected port if disconnected.
     * @return Whether it was triggered this frame.
     */
    bool isPadTriggerUiRightByPort(s32 port) {
        return isPadTriggerRightOrStick(tryGetConnectCheckedPort(port));
    }

    /**
     * @brief Checks whether the first active control user triggered UI up.
     * @param accessor Accessor used to find the first active control user.
     * @return Whether it was triggered this frame.
     */
    bool isPadTriggerUiUp(GameDataHolderAccessor accessor) {
        return isPadTriggerUiUpByPort(calcPadPortByFirstActiveUser(accessor));
    }

    /**
     * @brief Checks whether the controller triggered UI up.
     * @param port Controller port; replaced by the first connected port if disconnected.
     * @return Whether it was triggered this frame.
     */
    bool isPadTriggerUiUpByPort(s32 port) {
        return isPadTriggerUpOrStick(tryGetConnectCheckedPort(port));
    }

    /**
     * @brief Checks whether the first active control user triggered UI down.
     * @param accessor Accessor used to find the first active control user.
     * @return Whether it was triggered this frame.
     */
    bool isPadTriggerUiDown(GameDataHolderAccessor accessor) {
        return isPadTriggerUiDownByPort(calcPadPortByFirstActiveUser(accessor));
    }

    /**
     * @brief Checks whether the controller triggered UI down.
     * @param port Controller port; replaced by the first connected port if disconnected.
     * @return Whether it was triggered this frame.
     */
    bool isPadTriggerUiDownByPort(s32 port) {
        return isPadTriggerDownOrStick(tryGetConnectCheckedPort(port));
    }

    /**
     * @brief Falls back to the first connected pad when the given port is disconnected.
     * @param port Controller port.
     * @return The port itself if connected, else the first connected port of the pad port list, else the port.
     */
    s32 tryGetConnectCheckedPort(s32 port) {
        if (!al::isPadConnected(port)) {
            s32 connectedPort = al::tryGetPortFirstConnected(GameDataConst::getPadPortList(),
                                                             GameDataConst::getPadPortListNum());
            return connectedPort < 0 ? port : connectedPort;
        }

        return port;
    }

    /**
     * @brief Finds the first connected Npad controller among ports 1 to 4.
     * @return The first connected port, or the main controller port if none is connected.
     */
    s32 tryGetRawControllerPortFirstConnected() {
        for (s32 port = 1; port <= 4; port++) {
            sead::Controller* controller = sead::ControllerMgr::instance()->getController(port);
            if (controller == nullptr) {
                continue;
            }

            al::NpadController* npadController = sead::DynamicCast<al::NpadController>(controller);
            if (npadController != nullptr && npadController->isConnected()) {
                return port;
            }
        }

        return al::getMainControllerPort();
    }

    /**
     * @brief Checks whether gyro touch control can be used with the controller.
     * @param port Controller port.
     * @return Whether the controller is not in handheld mode.
     */
    bool isEnableGyroTouchControl(s32 port) {
        return !al::isPadTypeHandheld(port);
    }

    /**
     * @brief Checks whether the start (plus) button was triggered.
     * @param port Controller port; replaced by the first connected port if disconnected.
     * @return Whether it was triggered this frame.
     */
    bool isPadTriggerStart(s32 port) {
        return al::isPadTriggerPlus(tryGetConnectCheckedPort(port));
    }

    /**
     * @brief Checks whether the select (minus) button was triggered.
     * @param port Controller port; replaced by the first connected port if disconnected.
     * @return Whether it was triggered this frame.
     */
    bool isPadTriggerSelect(s32 port) {
        return al::isPadTriggerMinus(tryGetConnectCheckedPort(port));
    }

    /**
     * @brief Checks whether a window close input (A or a touch) was triggered.
     * @param port Controller port; replaced by the first connected port if disconnected.
     * @return Whether it was triggered this frame.
     */
    bool isPadTriggerWindowClose(s32 port) {
        s32 checkedPort = tryGetConnectCheckedPort(port);
        s32 touchPort = calcTouchPanelPortByPortNum(checkedPort);
        return al::isPadTriggerA(checkedPort) || al::isPadTriggerTouch(touchPort);
    }

    /**
     * @brief Checks whether the pad uses the Wii Remote layout without a sub controller.
     * @param port Controller port (unused).
     * @return Always false.
     */
    bool isPadLayoutWiiRemoteWithoutSubController(s32 port) {
        return false;
    }

    /**
     * @brief Checks whether the decide (A) button was triggered.
     * @param port Controller port.
     * @return Whether it was triggered this frame.
     */
    bool isPadTriggerDecide(s32 port) {
        return al::isPadTriggerA(port);
    }

    /**
     * @brief Checks whether the cancel (B) button was triggered.
     * @param port Controller port.
     * @return Whether it was triggered this frame.
     */
    bool isPadTriggerCancel(s32 port) {
        return al::isPadTriggerB(port);
    }

    /**
     * @brief Checks whether left was triggered on the directional buttons or the left stick.
     * @param port Controller port.
     * @return Whether it was triggered.
     */
    bool isPadTriggerLeftOrStick(s32 port) {
        return al::isPadTriggerLeft(port) || al::isPadTriggerLeftLeftStick(port);
    }

    /**
     * @brief Checks whether right was triggered on the directional buttons or the left stick.
     * @param port Controller port.
     * @return Whether it was triggered.
     */
    bool isPadTriggerRightOrStick(s32 port) {
        return al::isPadTriggerRight(port) || al::isPadTriggerRightLeftStick(port);
    }

    /**
     * @brief Checks whether up was triggered on the directional buttons or the left stick.
     * @param port Controller port.
     * @return Whether it was triggered.
     */
    bool isPadTriggerUpOrStick(s32 port) {
        return al::isPadTriggerUp(port) || al::isPadTriggerUpLeftStick(port);
    }

    /**
     * @brief Checks whether down was triggered on the directional buttons or the left stick.
     * @param port Controller port.
     * @return Whether it was triggered.
     */
    bool isPadTriggerDownOrStick(s32 port) {
        return al::isPadTriggerDown(port) || al::isPadTriggerDownLeftStick(port);
    }

    /**
     * @brief Checks whether left was held on the directional buttons or the left stick.
     * @param port Controller port.
     * @return Whether it was held.
     */
    bool isPadHoldLeftOrStick(s32 port) {
        return al::isPadHoldLeft(port) || al::isPadHoldLeftLeftStick(port);
    }

    /**
     * @brief Checks whether right was held on the directional buttons or the left stick.
     * @param port Controller port.
     * @return Whether it was held.
     */
    bool isPadHoldRightOrStick(s32 port) {
        return al::isPadHoldRight(port) || al::isPadHoldRightLeftStick(port);
    }

    /**
     * @brief Checks whether up was held on the directional buttons or the left stick.
     * @param port Controller port.
     * @return Whether it was held.
     */
    bool isPadHoldUpOrStick(s32 port) {
        return al::isPadHoldUp(port) || al::isPadHoldUpLeftStick(port);
    }

    /**
     * @brief Checks whether down was held on the directional buttons or the left stick.
     * @param port Controller port.
     * @return Whether it was held.
     */
    bool isPadHoldDownOrStick(s32 port) {
        return al::isPadHoldDown(port) || al::isPadHoldDownLeftStick(port);
    }

    /**
     * @brief Checks whether the UI L button was triggered.
     * @param port Controller port; replaced by the first connected port if disconnected.
     * @return Whether it was triggered this frame.
     */
    bool isPadTriggerUiLByPort(s32 port) {
        return al::isPadTriggerL(tryGetConnectCheckedPort(port));
    }

    /**
     * @brief Checks whether the UI R button was triggered.
     * @param port Controller port; replaced by the first connected port if disconnected.
     * @return Whether it was triggered this frame.
     */
    bool isPadTriggerUiRByPort(s32 port) {
        return al::isPadTriggerR(tryGetConnectCheckedPort(port));
    }

    /**
     * @brief Checks whether the UI inventory button (X, or up on a single Joy-Con) was triggered.
     * @param port Controller port; replaced by the first connected port if disconnected.
     * @return Whether it was triggered this frame.
     */
    bool isPadTriggerUiInventoryByPort(s32 port) {
        s32 checkedPort = tryGetConnectCheckedPort(port);
        if (al::isPadTypeJoySingle(checkedPort)) {
            return al::isPadTriggerX(checkedPort);
        }

        return al::isPadTriggerUp(checkedPort);
    }

    /**
     * @brief Checks whether the UI map button was triggered.
     * @param port Controller port; replaced by the first connected port if disconnected.
     * @return Whether it was triggered this frame.
     */
    bool isPadTriggerUiMapByPort(s32 port) {
        s32 checkedPort = tryGetConnectCheckedPort(port);
        if (al::isPadTypeJoySingle(checkedPort)) {
            if (al::isPadTypeJoyRight(checkedPort)) {
                return al::isPadTriggerPlus(checkedPort);
            }

            return al::isPadTriggerMinus(checkedPort);
        }

        return al::isPadTriggerRight(checkedPort) || al::isPadTriggerMinus(checkedPort);
    }

    /**
     * @brief Calculates the vertical UI scroll input.
     * @param port Controller port; replaced by the first connected port if disconnected.
     * @return The scroll amount: +1 / -1 for the directional buttons, else the left stick Y.
     */
    f32 getPadUiScrollY(s32 port) {
        s32 checkedPort = tryGetConnectCheckedPort(port);
        f32 scrollY = 0.0f;
        if (al::isPadHoldLeftStick(checkedPort)) {
            scrollY = al::getLeftStick(checkedPort).y;
        }

        if (al::isPadHoldUp(checkedPort)) {
            return 1.0f;
        }

        if (al::isPadHoldDown(checkedPort)) {
            return -1.0f;
        }

        return scrollY;
    }
};  // namespace rc

/**
 * @brief Checks whether the map can be scrolled with the controller.
 * @param port Controller port.
 * @return False on a single Joy-Con while A is held (A switches it to zoom).
 */
bool isEnablePadUiMapScrollInner(s32 port) {
    if (al::isPadTypeJoySingle(port)) {
        return !al::isPadHoldA(port);
    }

    return true;
}

/**
 * @brief Checks whether the map can be zoomed with the controller.
 * @param port Controller port.
 * @return True unless on a single Joy-Con without A held.
 */
bool isEnablePadUiMapZoomInner(s32 port) {
    if (al::isPadTypeJoySingle(port)) {
        return al::isPadHoldA(port);
    }

    return true;
}

namespace rc {
    /**
     * @brief Checks whether the map can be zoomed with the controller.
     * @param port Controller port; replaced by the first connected port if disconnected.
     * @return Whether zoom input is enabled.
     */
    bool isEnablePadUiMapZoom(s32 port) {
        return isEnablePadUiMapZoomInner(tryGetConnectCheckedPort(port));
    }

    /**
     * @brief Gets the map scroll input.
     * @param port Controller port; replaced by the first connected port if disconnected.
     * @return The left stick, or zero while scrolling is disabled.
     */
    sead::Vector2f getPadUiMapScrollByPort(s32 port) {
        s32 checkedPort = tryGetConnectCheckedPort(port);
        if (isEnablePadUiMapScrollInner(checkedPort)) {
            return al::getLeftStick(checkedPort);
        }

        return sead::Vector2f::zero;
    }

    /**
     * @brief Gets the map zoom input.
     * @param port Controller port; replaced by the first connected port if disconnected.
     * @return The zoom stick (left on a single Joy-Con, else right), or zero while zooming is disabled.
     */
    sead::Vector2f getPadUiMapZoomByPort(s32 port) {
        s32 checkedPort = tryGetConnectCheckedPort(port);
        if (isEnablePadUiMapZoomInner(checkedPort)) {
            if (al::isPadTypeJoySingle(checkedPort)) {
                return al::getLeftStick(checkedPort);
            }

            return al::getRightStick(checkedPort);
        }

        return sead::Vector2f::zero;
    }

    /**
     * @brief Checks whether the pad uses the Wii Remote layout.
     * @param port Controller port (unused).
     * @return Always false.
     */
    bool isPadLayoutWiiRemote(s32 port) {
        return false;
    }

    /**
     * @brief Checks on the raw controller, bypassing replays, whether the A button was triggered.
     * @param port Controller port.
     * @return The raw button state.
     */
    bool isRawPadTriggerA(s32 port) {
        return getRawController(port)->isTrig(sead::Controller::cPadMask_A);
    }

    /**
     * @brief Checks on the raw controller, bypassing replays, whether the B button was triggered.
     * @param port Controller port.
     * @return The raw button state.
     */
    bool isRawPadTriggerB(s32 port) {
        return getRawController(port)->isTrig(sead::Controller::cPadMask_B);
    }

    /**
     * @brief Checks on the raw controller, bypassing replays, whether the 2 button was triggered.
     * @param port Controller port.
     * @return The raw button state.
     */
    bool isRawPadTrigger2(s32 port) {
        return getRawController(port)->isTrig(sead::Controller::cPadMask_2);
    }

    /**
     * @brief Checks on the raw controller, bypassing replays, whether the L button was triggered.
     * @param port Controller port.
     * @return The raw button state.
     */
    bool isRawPadTriggerL(s32 port) {
        return getRawController(port)->isTrig(sead::Controller::cPadMask_L);
    }

    /**
     * @brief Checks on the raw controller, bypassing replays, whether the X button was triggered.
     * @param port Controller port.
     * @return The raw button state.
     */
    bool isRawPadTriggerX(s32 port) {
        return getRawController(port)->isTrig(sead::Controller::cPadMask_X);
    }

    /**
     * @brief Checks on the raw controller, bypassing replays, whether the Y button was triggered.
     * @param port Controller port.
     * @return The raw button state.
     */
    bool isRawPadTriggerY(s32 port) {
        return getRawController(port)->isTrig(sead::Controller::cPadMask_Y);
    }

    /**
     * @brief Checks on the raw controller, bypassing replays, whether the X button is held.
     * @param port Controller port.
     * @return The raw button state.
     */
    bool isRawPadHoldX(s32 port) {
        return getRawController(port)->isHold(sead::Controller::cPadMask_X);
    }

    /**
     * @brief Checks on the raw controller, bypassing replays, whether the Y button is held.
     * @param port Controller port.
     * @return The raw button state.
     */
    bool isRawPadHoldY(s32 port) {
        return getRawController(port)->isHold(sead::Controller::cPadMask_Y);
    }

    /**
     * @brief Checks on the raw controller, bypassing replays, whether the L button is held.
     * @param port Controller port.
     * @return The raw button state.
     */
    bool isRawPadHoldL(s32 port) {
        return getRawController(port)->isHold(sead::Controller::cPadMask_L);
    }

    /**
     * @brief Checks on the raw controller, bypassing replays, whether the touch panel was touched.
     * @param port Controller port.
     * @return The raw button state.
     */
    bool isRawPadTriggerTouch(s32 port) {
        return getRawController(port)->isTrig(sead::Controller::cPadMask_Touch);
    }

    /**
     * @brief Checks on the raw controller, bypassing replays, whether the touch panel was released.
     * @param port Controller port.
     * @return The raw button state.
     */
    bool isRawPadReleaseTouch(s32 port) {
        return getRawController(port)->isRelease(sead::Controller::cPadMask_Touch);
    }

    /**
     * @brief Checks on the raw controller, bypassing replays, whether the touch panel is held.
     * @param port Controller port.
     * @return The raw button state.
     */
    bool isRawPadHoldTouch(s32 port) {
        return getRawController(port)->isHold(sead::Controller::cPadMask_Touch);
    }

    /**
     * @brief Calculates the raw touch position in layout coordinates (origin at the center, Y up).
     * @param pPos Output layout position.
     * @param port Controller port.
     */
    void calcRawLayoutTouchPos(sead::Vector2f* pPos, s32 port) {
        const sead::Vector2f& rPointer = getRawController(port)->getPointer();
        f32 x = rPointer.x;
        f32 y = rPointer.y;
        f32 width = al::getLayoutDisplayWidth();
        f32 height = al::getLayoutDisplayHeight();
        pPos->x = x - width * 0.5f;
        pPos->y = -(y - height * 0.5f);
    }

    /**
     * @brief Checks on the raw controller whether the UI decide (A) button was triggered.
     * @param port Controller port.
     * @param isUnused Unused.
     * @return Whether A was triggered.
     */
    bool isRawPadTriggerUiDecideByPort(s32 port, bool isUnused) {
        return getRawController(port)->isTrig(sead::Controller::cPadMask_A);
    }

    /**
     * @brief Checks on the raw controller whether UI up was triggered (buttons or left stick).
     * @param port Controller port.
     * @return Whether it was triggered.
     */
    bool isRawPadTriggerUiUpByPort(s32 port) {
        return getRawController(port)->isTrig(sead::Controller::cPadMask_Up |
                                              sead::Controller::cPadMask_LeftStickUp);
    }

    /**
     * @brief Checks on the raw controller whether UI down was triggered (buttons or left stick).
     * @param port Controller port.
     * @return Whether it was triggered.
     */
    bool isRawPadTriggerUiDownByPort(s32 port) {
        return getRawController(port)->isTrig(sead::Controller::cPadMask_Down |
                                              sead::Controller::cPadMask_LeftStickDown);
    }

    /**
     * @brief Checks on the raw controller whether UI left was triggered (buttons or left stick).
     * @param port Controller port.
     * @return Whether it was triggered.
     */
    bool isRawPadTriggerUiLeftByPort(s32 port) {
        return getRawController(port)->isTrig(sead::Controller::cPadMask_Left |
                                              sead::Controller::cPadMask_LeftStickLeft);
    }

    /**
     * @brief Checks on the raw controller whether UI right was triggered (buttons or left stick).
     * @param port Controller port.
     * @return Whether it was triggered.
     */
    bool isRawPadTriggerUiRightByPort(s32 port) {
        return getRawController(port)->isTrig(sead::Controller::cPadMask_Right |
                                              sead::Controller::cPadMask_LeftStickRight);
    }

    /**
     * @brief Checks on the raw controller whether the plus button was triggered.
     * @param port Controller port.
     * @return The raw button state.
     */
    bool isRawPadTriggerPlus(s32 port) {
        return getRawController(port)->isTrig(sead::Controller::cPadMask_Plus);
    }

    /**
     * @brief Checks on the raw controller whether the minus button was triggered.
     * @param port Controller port.
     * @return The raw button state.
     */
    bool isRawPadTriggerMinus(s32 port) {
        return getRawController(port)->isTrig(sead::Controller::cPadMask_Minus);
    }

    /**
     * @brief Forces players to connect (no-op in this version).
     * @param writer Writer of the game data holder (unused).
     * @param playerNum Number of players (unused).
     */
    void forceConnectPlayers(GameDataHolderWriter writer, s32 playerNum) {}

    /**
     * @brief Switches the game pad system between single and multi player mode.
     * @param pSystem Game pad system to switch.
     * @param writer Writer of the game data holder (unused).
     * @param playerNum Number of players; two or more selects multi player mode.
     * @param isAnyController Whether any controller may be used in single player mode.
     */
    void setMultiPlayerMode(al::GamePadSystem* pSystem, GameDataHolderWriter writer, s32 playerNum,
                            bool isAnyController) {
        bool isPrevSingle = pSystem->getMaxPlayerNum() < 2;
        bool isSingle;
        if (playerNum >= 2) {
            pSystem->changeMultiPlayMode(playerNum, 1);
            isSingle = false;
        } else {
            pSystem->changeSinglePlayMode(isAnyController);
            isSingle = true;
        }

        if (isSingle != isPrevSingle) {
            pSystem->setDisconnectFrame(0);
            pSystem->forceImmediateDisconnect(true);
        }
    }

    /**
     * @brief Checks whether the game pad system is in multi player mode.
     * @return Whether more than one player can play.
     */
    bool isMultiPlayerMode() {
        return getGamePadSystem()->getMaxPlayerNum() > 1;
    }

    /**
     * @brief Enables or disables the controller connect checker.
     * @param isDisabled Whether the checker is disabled.
     */
    void setControllerConnectDisabled(bool isDisabled) {
        getGamePadSystem()->disableControllerConnectChecker(isDisabled);
    }

    /**
     * @brief Enables or disables the controller support applet.
     * @param isDisabled Whether the applet is disabled.
     */
    void setControllerAppletDisabled(bool isDisabled) {
        getGamePadSystem()->disableControllerApplet(isDisabled);
    }

    /**
     * @brief Gets the number of players.
     * @return 2 in multi player mode, else 1.
     */
    s64 getNumPlayers() {
        return isMultiPlayerMode() ? 2 : 1;
    }

    /**
     * @brief Gets the Npad style of the controller at a port.
     * @param port Controller port.
     * @return The controller style, or cStyle_Invalid if the port holds no Npad controller.
     */
    s32 getControllerStyle(s32 port) {
        al::NpadController* controller =
            sead::DynamicCast<al::NpadController>(sead::ControllerMgr::instance()->getController(port));
        if (controller != nullptr) {
            return controller->getStyle();
        }

        return sead::NinJoyNpadDevice::cStyle_Invalid;
    }

    /**
     * @brief Checks whether the controller assignment changed.
     * @return Whether the game pad system reported a pad state change.
     */
    bool isControllerAssignmentChanged() {
        return getGamePadSystem()->isChangedPadState();
    }

    /**
     * @brief Gets the direction held on the directional buttons.
     * @param pDir Output direction.
     * @param port Controller port.
     */
    void getPadCrossDir(sead::Vector2f* pDir, s32 port) {
        pDir->x = 0.0f;
        pDir->y = 0.0f;

        if (al::isPadHoldUp(port)) {
            pDir->y = 1.0f;
        }

        if (al::isPadHoldDown(port)) {
            pDir->y = -1.0f;
        }

        if (al::isPadHoldLeft(port)) {
            pDir->x = -1.0f;
        }

        if (al::isPadHoldRight(port)) {
            pDir->x = 1.0f;
        }
    }

    /**
     * @brief Finds the first port in a mask with camera stick input and gets its direction.
     * @param pDir Output stick direction.
     * @param unused Unused.
     * @param portMask Bit mask of the ports to check.
     * @param isIgnoreHoldA Whether single Joy-Cons need not hold A to move the camera.
     * @param isUseLeftStick Whether the left stick of a full controller also moves the camera.
     * @param pPort Output port that provided the input; left untouched if none did.
     * @return The port that provided the input, or -1 if none.
     */
    s32 tryGetCameraInputStickDirMask(sead::Vector2f* pDir, s32 unused, u32 portMask,
                                      bool isIgnoreHoldA, bool isUseLeftStick, s32* pPort) {
        for (s32 port = 0; port < cCameraInputPortNum; port++) {
            if ((portMask & (1 << port)) == 0) {
                continue;
            }

            if (al::isPadTypeJoySingle(port)) {
                if (!isIgnoreHoldA && !al::isPadHoldA(port)) {
                    continue;
                }

                if (al::isPadHoldLeftStick(port)) {
                    pDir->set(al::getLeftStick(port));
                    *pPort = port;
                    return port;
                }
            } else {
                if (al::isPadHoldRightStick(port)) {
                    pDir->set(al::getRightStick(port));
                    *pPort = port;
                    return port;
                }

                if (isUseLeftStick && al::isPadHoldLeftStick(port)) {
                    pDir->set(al::getLeftStick(port));
                    *pPort = port;
                    return port;
                }

                if (al::isPadHoldUp(port) || al::isPadHoldDown(port) || al::isPadHoldRight(port) ||
                    al::isPadHoldLeft(port)) {
                    getPadCrossDir(pDir, port);
                    *pPort = port;
                    return port;
                }
            }
        }

        return -1;
    }

    /**
     * @brief Requests the controller support applet by forcing an immediate disconnect.
     */
    void forceControllerApplet() {
        getGamePadSystem()->requestForceImmediateDisconnect();
    }

    /**
     * @brief Sets the 2P assist mode of the game pad system.
     * @param isAssist Whether 2P assist mode is enabled.
     * @param isForceDisconnect Whether to force a disconnect when switching.
     */
    void set2PAssistMode(bool isAssist, bool isForceDisconnect) {
        getGamePadSystem()->setAssistMode(isAssist, isForceDisconnect);
    }

    /**
     * @brief Sets the 2P assist flag after the controller applet was cancelled.
     * @param isAssist Whether 2P assist mode is enabled.
     */
    void setAppletCancel2PAssistMode(bool isAssist) {
        getGamePadSystem()->set2PAssistMode(isAssist);
    }

    /**
     * @brief Registers the software keyboard as the cancel user of the game pad system.
     * @param pCancel Cancel user to register.
     */
    void setSoftwareKeyboard(al::IUseCancel* pCancel) {
        getGamePadSystem()->setSoftwareKeyboard(pCancel);
    }

    /**
     * @brief Configures the game pad system for the top menu (one single Joy-Con player).
     */
    void setTopMenuPlayerMode() {
        al::GamePadSystem* system = getGamePadSystem();
        system->setMaxNpadNum(1);
        system->initSingleJoycon();
        system->changeTopMenuPlayMode();
    }

    /**
     * @brief Configures the game pad system for Super Mario 3D World (up to four players).
     */
    void set3dWorldPlayerMode() {
        al::GamePadSystem* system = getGamePadSystem();
        system->setMaxNpadNum(4);
        system->changeMultiPlayMode(4, 1);
        system->setIsEnableAutoHandheld(true);
    }
};  // namespace rc

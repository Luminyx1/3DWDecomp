#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
    class GamePadSystem;
    class IUseCancel;
};  // namespace al

class GameDataHolderAccessor;
class GameDataHolderWriter;

namespace rc {
    bool isPadTriggerUiDecide(GameDataHolderAccessor accessor);
    bool isPadTriggerUiDecideByPort(s32 port);
    bool isPadTriggerUiCancel(GameDataHolderAccessor accessor);
    bool isPadTriggerUiCancelByPort(s32 port);
    bool isPadTriggerUiLeft(GameDataHolderAccessor accessor);
    bool isPadTriggerUiLeftByPort(s32 port);
    bool isPadTriggerUiRight(GameDataHolderAccessor accessor);
    bool isPadTriggerUiRightByPort(s32 port);
    bool isPadTriggerUiUp(GameDataHolderAccessor accessor);
    bool isPadTriggerUiUpByPort(s32 port);
    bool isPadTriggerUiDown(GameDataHolderAccessor accessor);
    bool isPadTriggerUiDownByPort(s32 port);
    s32 tryGetConnectCheckedPort(s32 port);
    s32 tryGetRawControllerPortFirstConnected();
    bool isEnableGyroTouchControl(s32 port);
    bool isPadTriggerStart(s32 port);
    bool isPadTriggerSelect(s32 port);
    bool isPadTriggerWindowClose(s32 port);
    bool isPadLayoutWiiRemoteWithoutSubController(s32 port);
    bool isPadTriggerDecide(s32 port);
    bool isPadTriggerCancel(s32 port);
    bool isPadTriggerLeftOrStick(s32 port);
    bool isPadTriggerRightOrStick(s32 port);
    bool isPadTriggerUpOrStick(s32 port);
    bool isPadTriggerDownOrStick(s32 port);
    bool isPadHoldLeftOrStick(s32 port);
    bool isPadHoldRightOrStick(s32 port);
    bool isPadHoldUpOrStick(s32 port);
    bool isPadHoldDownOrStick(s32 port);
    bool isPadTriggerUiLByPort(s32 port);
    bool isPadTriggerUiRByPort(s32 port);
    bool isPadTriggerUiInventoryByPort(s32 port);
    bool isPadTriggerUiMapByPort(s32 port);
    f32 getPadUiScrollY(s32 port);
    bool isEnablePadUiMapZoom(s32 port);
    sead::Vector2f getPadUiMapScrollByPort(s32 port);
    sead::Vector2f getPadUiMapZoomByPort(s32 port);
    bool isPadLayoutWiiRemote(s32 port);
    bool isRawPadTriggerA(s32 port);
    bool isRawPadTriggerB(s32 port);
    bool isRawPadTrigger2(s32 port);
    bool isRawPadTriggerL(s32 port);
    bool isRawPadTriggerX(s32 port);
    bool isRawPadTriggerY(s32 port);
    bool isRawPadHoldX(s32 port);
    bool isRawPadHoldY(s32 port);
    bool isRawPadHoldL(s32 port);
    bool isRawPadTriggerTouch(s32 port);
    bool isRawPadReleaseTouch(s32 port);
    bool isRawPadHoldTouch(s32 port);
    void calcRawLayoutTouchPos(sead::Vector2f* pPos, s32 port);
    bool isRawPadTriggerUiDecideByPort(s32 port, bool isUnused);
    bool isRawPadTriggerUiUpByPort(s32 port);
    bool isRawPadTriggerUiDownByPort(s32 port);
    bool isRawPadTriggerUiLeftByPort(s32 port);
    bool isRawPadTriggerUiRightByPort(s32 port);
    bool isRawPadTriggerPlus(s32 port);
    bool isRawPadTriggerMinus(s32 port);
    void forceConnectPlayers(GameDataHolderWriter writer, s32 playerNum);
    void setMultiPlayerMode(al::GamePadSystem* pSystem, GameDataHolderWriter writer, s32 playerNum,
                            bool isAnyController);
    bool isMultiPlayerMode();
    void setControllerConnectDisabled(bool isDisabled);
    void setControllerAppletDisabled(bool isDisabled);
    s64 getNumPlayers();
    s32 getControllerStyle(s32 port);
    bool isControllerAssignmentChanged();
    void getPadCrossDir(sead::Vector2f* pDir, s32 port);
    s32 tryGetCameraInputStickDirMask(sead::Vector2f* pDir, s32 unused, u32 portMask,
                                      bool isIgnoreHoldA, bool isUseLeftStick, s32* pPort);
    void forceControllerApplet();
    void set2PAssistMode(bool isAssist, bool isForceDisconnect);
    void setAppletCancel2PAssistMode(bool isAssist);
    void setSoftwareKeyboard(al::IUseCancel* pCancel);
    void setTopMenuPlayerMode();
    void set3dWorldPlayerMode();
};  // namespace rc

bool isEnablePadUiMapScrollInner(s32 port);
bool isEnablePadUiMapZoomInner(s32 port);

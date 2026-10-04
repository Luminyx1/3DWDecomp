#pragma once

#include <basis/seadTypes.h>

namespace al {
    class HitSensor;
    class LiveActor;
    class ScreenPointer;
};

class GameDataHolderAccessor;
class GameDataHolderWriter;

namespace rc {
    s32 getControlUserNumMax();
    s32 getPadPortByUserId(s32 userId);
    s32 calcControlUserIdByPortNum(s32 port);
    s32 calcTouchPanelPortByPortNum(s32 port);
    void changeUserPort(GameDataHolderWriter writer, s32 userIdA, s32 userIdB);
    s32 calcPadPortByFirstActiveUser(GameDataHolderAccessor accessor);
    s32 findActiveUserIdList(s32* pUserIds, GameDataHolderAccessor accessor);
    s32 tryCalcPadPortByFirstActiveUser(GameDataHolderAccessor accessor);
    s32 calcCharacterTypeNumMax(GameDataHolderAccessor accessor);
    s32 getActiveControlUserNum(GameDataHolderAccessor accessor);
    u64 getActiveInputPortList(GameDataHolderAccessor accessor);
    s32 getControlUserPortNumber(GameDataHolderAccessor accessor, s32 userId);
    s32 calcActiveUserNumInOrder(GameDataHolderAccessor accessor, s32 userId);
    bool isActiveControlUser(GameDataHolderAccessor accessor, s32 userId);
    bool isDeadControlUserInStage(GameDataHolderAccessor accessor, s32 userId);
    void resetDeadFlagControlUserInStage(GameDataHolderWriter writer, s32 userId);
    bool isExistDeadlUserInStage(GameDataHolderAccessor accessor);
    s32 getActiveControlUserFirst(GameDataHolderAccessor accessor);
    s32 getControlUserCharacterType(GameDataHolderAccessor accessor, s32 userId);
    const char* getControlUserCharacterName(GameDataHolderAccessor accessor, s32 userId);
    const char16_t* getControlUserCharacterPictureFont(GameDataHolderAccessor accessor, s32 userId);
    s32 getControlUserFigureType(GameDataHolderAccessor accessor, s32 userId);
    void setControlUserFigureType(GameDataHolderWriter writer, s32 userId, s32 figureType);
    s32 calcControlUserDisplayOrder(GameDataHolderAccessor accessor, s32 userId);
    s32 tryCalcControlUserIdFromPortNum(GameDataHolderAccessor accessor, s32 port);
    s32 calcControlUserIdFromPortNum(GameDataHolderAccessor accessor, s32 port);
    s32 calcControlUserIdByCharacterType(GameDataHolderAccessor accessor, s32 characterType,
                                         bool isAllowUserSlot);
    s32 tryCalcControlUserIdByCharacterType(GameDataHolderAccessor accessor, s32 characterType,
                                            bool isAllowUserSlot);
    s32 tryFindControlUserId(const al::LiveActor* pActor);
    s32 findControlUserId(const al::LiveActor* pActor);
    s32 findControlUserId(const al::HitSensor* pSensor);
    s32 tryFindControlUserId(const al::HitSensor* pSensor);
    s32 findRelativeControlUserId(al::HitSensor* pSensor);
    s32 tryFindRelativeControlUserId(al::HitSensor* pSensor);
    u32 tryFindRelativeControlUserIdBitFlag(al::HitSensor* pSensor);
    s32 tryGetRelativeControlUserFigureType(GameDataHolderAccessor accessor, al::HitSensor* pSensor);
    s32 tryFindRelativeControlUserId(const al::LiveActor* pActor, al::ScreenPointer* pPointer);
    void set2PAssistMode(bool isAssist, bool isForceDisconnect);
};

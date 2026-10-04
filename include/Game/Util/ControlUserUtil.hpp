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
    s32 tryFindRelativeControlUserId(const al::LiveActor *, al::ScreenPointer *);
    s32 getControlUserNumMax();
    bool isActiveControlUser(GameDataHolderAccessor accessor, s32 userId);
    s32 getActiveControlUserFirst(GameDataHolderAccessor accessor);
    s32 getActiveControlUserNum(GameDataHolderAccessor accessor);
    s32 getControlUserPortNumber(GameDataHolderAccessor accessor, s32 userId);
    s32 getControlUserCharacterType(GameDataHolderAccessor accessor, s32 userId);
    const char* getControlUserCharacterName(GameDataHolderAccessor accessor, s32 characterType);
    void setControlUserFigureType(GameDataHolderWriter writer, s32 userId, s32 figureType);
    s32 calcPadPortByFirstActiveUser(GameDataHolderAccessor accessor);
    void set2PAssistMode(bool isAssist, bool isForceDisconnect);
};
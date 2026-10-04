#pragma once

#include <basis/seadTypes.h>

namespace al {
    class HitSensor;
    class LiveActor;
    class ScreenPointer;
};

class GameDataHolderAccessor;

namespace rc {
    s32 tryFindRelativeControlUserId(const al::LiveActor *, al::ScreenPointer *);
    s32 getControlUserNumMax();
    bool isActiveControlUser(GameDataHolderAccessor accessor, s32 userId);
    s32 getActiveControlUserFirst(GameDataHolderAccessor accessor);
};
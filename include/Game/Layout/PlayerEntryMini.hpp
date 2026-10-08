#pragma once

#include <basis/seadTypes.h>

#include "Library/Layout/LayoutActor.hpp"

namespace al {
class LayoutInitInfo;
class PlayerHolder;
}  // namespace al

class GameDataHolder;
class PlayerEntryItem;

/// Small player-entry window listing one PlayerEntryItem per controller user.
class PlayerEntryMini : public al::LayoutActor {
public:
    PlayerEntryMini(const al::LayoutInitInfo& rInfo, GameDataHolder* pHolder,
                    al::PlayerHolder* pPlayerHolder, s32 itemNum);

    bool isPreStageWipe() const;
    bool isKinopioPreStageWipe() const;
    bool isKinopioAny() const;
    bool isKinopioStage() const;
    bool isStageScene() const;
    bool isCourseSelectScene() const;
    GameDataHolder* getGameDataHolder();
    s32 calcActiveItemNum() const;
    void onDecideItem(PlayerEntryItem* pItem);
    bool playerEntry(s32 userId, s32 characterType);
    void playerCancel(s32 userId);
    void requestStartLrAssignMode();
    void requestStopLrAssignMode();
};

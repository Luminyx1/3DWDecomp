#pragma once

#include "Library/Layout/LayoutActor.hpp"

namespace al {
class LayoutInitInfo;
class PlayerHolder;
}  // namespace al

class GameDataHolder;
class GreenStarKeeper;
class IllustItemKeeper;
class PlayerAliveWatcher;
class ProjectItemDirector;
class StageTimer;

/**
 * @brief Layouts of a stage: timer, counters and item stock.
 * @note Minimal declaration: only what the reconstructed code uses is declared so far.
 */
class StageSceneLayout : public al::LayoutActor {
public:
    StageSceneLayout(const al::LayoutInitInfo& rInfo, GameDataHolder* pGameDataHolder,
                     const char* pStageName, const GreenStarKeeper* pGreenStarKeeper,
                     const IllustItemKeeper* pIllustItemKeeper,
                     const al::PlayerHolder* pPlayerHolder,
                     const PlayerAliveWatcher* pPlayerAliveWatcher,
                     ProjectItemDirector* pItemDirector);

    void disableItemStock();
    void setStageKinopioBrigade();
    void startDemo(bool isHideTimer, bool isHideCounter);
    void endDemo(bool isShowTimer, bool isShowCounter);
    void endPause();
    void courseClear();

    /**
     * Gets the stage timer.
     * @return The stage timer.
     */
    StageTimer* getStageTimer() const { return mStageTimer; }

private:
    u8 _128[0x138 - sizeof(al::LayoutActor)];
    StageTimer* mStageTimer;  // 0x138
    u8 _140[0x180 - 0x140];
};

static_assert(sizeof(StageSceneLayout) == 0x180);

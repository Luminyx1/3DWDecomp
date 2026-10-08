#pragma once

#include <basis/seadTypes.h>

#include "Library/Layout/LayoutActor.hpp"

namespace al {
class CameraDirector;
class LayoutInitInfo;
class PlayerHolder;
class ScreenCaptureExecutor;
}  // namespace al

class CourseSelectDirector;
class GameDataHolder;
class RCSControlGuideBar;

/**
 * @brief Pause menu of the course select map.
 * @note Minimal declaration: only what the reconstructed code uses is declared so far.
 */
class PauseMenuMap : public al::LayoutActor {
public:
    PauseMenuMap(const al::LayoutInitInfo& rInfo, GameDataHolder* pGameDataHolder,
                 CourseSelectDirector* pDirector, al::CameraDirector* pCameraDirector,
                 al::PlayerHolder* pPlayerHolder, RCSControlGuideBar* pGuideBar,
                 al::ScreenCaptureExecutor* pScreenCaptureExecutor);

    void setControlGuideBar(RCSControlGuideBar* pGuideBar);
    void appear(s32 port, s32 worldId);
    bool isWait() const;
    void decideBack();
    bool isDecideBack() const;
    bool isDecideLeaveGame() const;
    bool isDecideGoToTitle() const;
    bool isEndGoToTitle() const;
    bool isDecideLoad() const;
    bool isEnd() const;
    void forceExit();
    bool isControllerChange() const;

    /**
     * Gets the pad port the menu is operated with.
     * @return The pad port.
     */
    s32 getPort() const { return mPort; }

private:
    u8 _121[0x138 - 0x121];
    s32 mPort;  // 0x138
    u8 _13c[0x1a8 - 0x13c];
};

static_assert(sizeof(PauseMenuMap) == 0x1a8);

#pragma once

#include <basis/seadTypes.h>

namespace al {
class GraphicsSystemInfo;
class LayoutInitInfo;
class SceneCameraInfo;
}  // namespace al

class CourseSelectDirector;
class GameDataHolder;
class RCSControlGuideBar;

/**
 * @brief Map menu (world map, star list and stamp list) of the course select map.
 * @note Minimal declaration: only what the reconstructed code uses is declared so far.
 */
class MapMenu {
public:
    MapMenu(const al::LayoutInitInfo& rInfo, const GameDataHolder* pGameDataHolder,
            al::SceneCameraInfo* pCameraInfo, CourseSelectDirector* pDirector,
            al::GraphicsSystemInfo* pGraphicsSystemInfo, RCSControlGuideBar* pGuideBar);

    void setControlGuideBar(RCSControlGuideBar* pGuideBar);
    void startAppear(s32 port, s32 worldId);
    bool isStartEnd() const;
    bool isEnd() const;
    void forceEnd();
    bool isDecideAny() const;
    void transitionOut(bool isRight);
    void transitionIn(bool isRight);
    bool isWorldJumpStart() const;
    bool isWorldJumpFinish() const;

    /**
     * Gets the world shown by the map.
     * @return The world id.
     */
    s32 getWorldId() const { return mWorldId; }

private:
    u8 _0[0x144];
    s32 mWorldId;  // 0x144
    u8 _148[0x180 - 0x148];
};

static_assert(sizeof(MapMenu) == 0x180);

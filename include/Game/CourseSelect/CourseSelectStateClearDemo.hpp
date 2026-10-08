#pragma once

#include <basis/seadTypes.h>

#include "Library/Nerve/NerveStateBase.hpp"

namespace al {
class ActorInitInfo;
class LayoutInitInfo;
class PlayerHolder;
class SceneCameraInfo;
struct GameSystemInfo;
}  // namespace al

class CourseSelectDirector;
class CourseSelectScene;

/**
 * @brief Course select scene state: Plays the demos shown after a course was cleared.
 * @note Minimal declaration: only what the reconstructed code uses is declared so far.
 */
class CourseSelectStateClearDemo : public al::NerveStateBase {
public:
    CourseSelectStateClearDemo(CourseSelectScene* pScene, const al::PlayerHolder* pPlayerHolder,
                               al::SceneCameraInfo* pCameraInfo, const al::LayoutInitInfo& rInfo);

private:
    u8 _pad[0x98 - sizeof(al::NerveStateBase)];
};

static_assert(sizeof(CourseSelectStateClearDemo) == 0x98);

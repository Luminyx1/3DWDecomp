#pragma once

#include "Library/Sequence/DemoDirector.hpp"

namespace al {
class PlayerHolder;
}  // namespace al

/**
 * @brief Project-side demo director: knows the names of the game's demo kinds.
 * @note Only the constructor and the demo-name getters are declared so far.
 */
class ProjectDemoDirector : public al::DemoDirector {
public:
    ProjectDemoDirector(al::PlayerHolder* pPlayerHolder, s32 maxActors);

    static const char* getDemoNameCamera();
    static const char* getDemoNameMovingCamera();
    static const char* getDemoNameIntro();
    static const char* getDemoNamePlayer();
    static const char* getDemoNameBinding();
    static const char* getDemoNameCutscene();
    static const char* getDemoNameInGameCutscene();
    static const char* getDemoNamePlayerCutscene();

private:
    u8 _f0[0x108 - 0xf0];
};

static_assert(sizeof(ProjectDemoDirector) == 0x108);

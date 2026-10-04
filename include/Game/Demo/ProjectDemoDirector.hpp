#pragma once

/**
 * @brief Project-side demo director: knows the names of the game's demo kinds.
 * @note Only the demo-name getters are declared so far.
 */
class ProjectDemoDirector {
public:
    static const char* getDemoNameCamera();
    static const char* getDemoNameMovingCamera();
    static const char* getDemoNameIntro();
    static const char* getDemoNamePlayer();
    static const char* getDemoNameBinding();
    static const char* getDemoNameCutscene();
    static const char* getDemoNameInGameCutscene();
    static const char* getDemoNamePlayerCutscene();
};

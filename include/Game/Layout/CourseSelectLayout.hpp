#pragma once

#include <basis/seadTypes.h>

class CourseSelectSceneLayout;

/**
 * @brief Holder of the course select map layouts.
 * @note Minimal declaration: only what the reconstructed code uses is declared so far.
 */
class CourseSelectLayout {
public:
    void startPause();
    void endPause();
    bool isDemo() const;
    void endDemo(bool isSkipAnim);

    /**
     * Gets the main layout of the course select map.
     * @return The scene layout.
     */
    CourseSelectSceneLayout* getSceneLayout() const { return mSceneLayout; }

private:
    u8 _0[0x10];
    CourseSelectSceneLayout* mSceneLayout;  // 0x10
};

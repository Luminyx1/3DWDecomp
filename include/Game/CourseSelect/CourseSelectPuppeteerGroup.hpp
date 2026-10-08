#pragma once

#include "Library/LiveActor/LiveActor.hpp"

class ICourseSelectActorController;

/**
 * @brief Actor owning the course-select puppeteers, one per control user.
 * @note Minimal declaration: only what the reconstructed code uses is declared so far.
 */
class CourseSelectPuppeteerGroup : public al::LiveActor {
public:
    void startLockAppearDemo(ICourseSelectActorController* pController);
    void startOpenGateKeeperDemo(ICourseSelectActorController* pController);
};

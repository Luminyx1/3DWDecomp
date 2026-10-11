#pragma once

class ICourseSelectActorController;

// Only the interface needed by pipe entrances is recovered here.
class CourseSelectPuppeteerGroup {
public:
    void startRouteDokan(ICourseSelectActorController* pController);
};

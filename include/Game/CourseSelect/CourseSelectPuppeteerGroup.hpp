#pragma once

class ICourseSelectActorController;

// Only the interface needed by course-select pipes is recovered here.
class CourseSelectPuppeteerGroup {
public:
    void startWorldWarpDokanDemo(ICourseSelectActorController* pController);
    bool isDemoEnd() const;
    void startRouteDokan(ICourseSelectActorController* pController);
};

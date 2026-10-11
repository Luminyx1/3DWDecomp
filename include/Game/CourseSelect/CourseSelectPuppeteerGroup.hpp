#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class CourseSelectPuppeteer;

class ICourseSelectActorController;

// Only the interface needed by course-select pipes is recovered here.
class CourseSelectPuppeteerGroup : public al::LiveActor {
public:
    void startDokanDemo(ICourseSelectActorController* pController);
    int getPuppeteerCount() const { return mPuppeteerCount; }
    CourseSelectPuppeteer* getPuppeteer(int index) const { return mpPuppeteers[index]; }
    void startWorldWarpDokanDemo(ICourseSelectActorController* pController);
    bool isDemoEnd() const;
    void startRouteDokan(ICourseSelectActorController* pController);
private:
    void* mpScene;
    ICourseSelectActorController* mpController;
    int mPuppeteerCount;
    CourseSelectPuppeteer** mpPuppeteers;
};
static_assert(sizeof(CourseSelectPuppeteerGroup) == 0x168);

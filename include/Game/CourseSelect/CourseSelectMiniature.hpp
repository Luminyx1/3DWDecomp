#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class StageDatabaseInfo;
class CourseSelectActorInfo;
class ICourseSelectActorController;

// Only the interface and fields used by flags and locks are recovered here.
class CourseSelectMiniature : public al::LiveActor {
public:
    int getCourseId() const;
    int getWorldId() const;
    const StageDatabaseInfo* getStageInfo() const;
    bool isGreenStarLock() const;
    const CourseSelectActorInfo* getCourseSelectActorInfo() const { return mpActorInfo; }
    ICourseSelectActorController* getController() const { return mpController; }
private:
    unsigned char _144[0x7c];
    CourseSelectActorInfo* mpActorInfo;
    unsigned char _1c8[8];
    ICourseSelectActorController* mpController;
    unsigned char _1d8[0x40];
};
static_assert(sizeof(CourseSelectMiniature) == 0x218);

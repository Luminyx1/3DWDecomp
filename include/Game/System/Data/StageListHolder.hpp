#pragma once
#include "System/Data/WorldInfo.hpp"
class StageListHolder {
  public:
    int getWorldNum() const;
    int getCourseTotalNum() const;
    StageDatabaseInfo* findStageDatabaseInfo(int courseId) const;
    WorldInfo* findWorldInfo(int worldId) const;
    const char* findStageName(int courseId) const;
    const char* findStageType(int courseId) const;
    int findStageTypeID(int courseId) const;

  private:
    WorldInfoList* mpWorldInfo;
    int mCastleCourse;
    int mSpecialCourse100;
    int mSpecialCourse101;
};
static_assert(sizeof(StageListHolder) == 0x18);

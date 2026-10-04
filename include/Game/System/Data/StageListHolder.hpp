#pragma once
#include "System/Data/WorldInfo.hpp"

class StageListHolder {
  public:
    StageListHolder();
    int findKoopaCastleCourseId(int worldId) const;
    int calcCourseId(int worldId, int stageId) const;
    int getWorldNum() const;
    int getCourseTotalNum() const;
    int tryCalcCourseId(int worldId, int stageId) const;
    bool tryCalcWorldAndStageId(int* pWorldId, int* pStageId, int courseId) const;
    void calcWorldAndStageId(int* pWorldId, int* pStageId, int courseId) const;
    const char* findStageName(int courseId) const;
    StageDatabaseInfo* findStageDatabaseInfo(int courseId) const;
    const char* findStageType(int courseId) const;
    int findStageTypeID(int courseId) const;
    WorldInfo* findWorldInfo(int worldId) const;

  private:
    WorldInfoList* mpWorldInfo = nullptr;
    int mKoopaCastleCourseId = 0;
    int mSpecialCourse100 = 0;
    int mSpecialCourse101 = 0;
};
static_assert(sizeof(StageListHolder) == 0x18);

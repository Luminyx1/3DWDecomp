#pragma once
#include "System/Data/StageDatabaseInfo.hpp"
namespace al {
class Resource;
}
class WorldInfo {
  public:
    WorldInfo();
    void init(int worldId, StageDatabaseInfo* pStages, int count);
    StageDatabaseInfo* getStageInfoByIndex(int index) const;
    int mWorldId;
    int mStageCount;
    StageDatabaseInfo* mpStages;
};
class WorldInfoList {
  public:
    WorldInfoList(int worldCount, int stageCount);
    WorldInfo* findWorldInfo(int worldId) const;
    StageDatabaseInfo* getStageInfoByIndex(int index) const;
    StageDatabaseInfo* findStageDatabaseInfo(int courseId) const;
    int mWorldCount;
    WorldInfo* mpWorlds;
    int mStageCount;
    StageDatabaseInfo* mpStages;
};
namespace StageInfoFunction {
WorldInfoList* createWorldInfoList(al::Resource* pResource);
} // namespace StageInfoFunction

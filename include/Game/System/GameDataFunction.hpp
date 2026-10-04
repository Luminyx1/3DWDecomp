#pragma once

#include "System/GameDataHolderAccessor.hpp"

class StageDatabaseInfo;
namespace GameDataFunction {
bool isSingleMode(GameDataHolderAccessor);
int getInvalidCourseId();
int getWorldNum(GameDataHolderAccessor accessor);
int getCourseTotalNum(GameDataHolderAccessor accessor);
StageDatabaseInfo* findStageDatabaseInfo(GameDataHolderAccessor accessor, int courseId);
int calcCourseId(GameDataHolderAccessor accessor, int worldId, int stageId);
}; // namespace GameDataFunction
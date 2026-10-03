#pragma once

#include "System/GameDataHolderAccessor.hpp"

class StageDatabaseInfo;
namespace GameDataFunction {
bool isSingleMode(GameDataHolderAccessor);
int getInvalidCourseId();
int getWorldNum(GameDataHolderAccessor accessor);
int getCourseTotalNum(GameDataHolderAccessor accessor);
StageDatabaseInfo* findStageDatabaseInfo(GameDataHolderAccessor accessor, int courseId);
}; // namespace GameDataFunction
#pragma once
#include "System/CourseInfo.hpp"
#include "System/GameDataHolderWriter.hpp"
class CourseInfoHolder {
  public:
    explicit CourseInfoHolder(int count);
    void initialize();
    void copy(const CourseInfoHolder* pOther);

  private:
    int mCount;
    CourseInfo* mpCourses;
};
namespace CourseInfoFunction {
bool setOpen(GameDataHolderWriter writer, int courseId);
bool setGreenStarLock(GameDataHolderWriter writer, int courseId);
void resetClearFlag(GameDataHolderWriter writer, int courseId);
void openCasinoRoom(GameDataHolderWriter writer, int courseId);
void openGoldenExpress(GameDataHolderWriter writer, int courseId);
bool isClose(GameDataHolderAccessor accessor, int courseId);
bool isOpen(GameDataHolderAccessor accessor, int courseId);
bool isClear(GameDataHolderAccessor accessor, int courseId);
bool isGreenStarLock(GameDataHolderAccessor accessor, int courseId);
void addMissCount(GameDataHolderWriter writer, int courseId);
bool isClearWithAssistBlock(GameDataHolderAccessor accessor, int courseId);
} // namespace CourseInfoFunction

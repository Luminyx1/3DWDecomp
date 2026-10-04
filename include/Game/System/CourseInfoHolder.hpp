#pragma once
#include "System/CourseInfo.hpp"
#include "System/GameDataHolderWriter.hpp"
namespace sead {
class ReadStream;
class WriteStream;
} // namespace sead
class CourseInfoHolder {
  public:
    explicit CourseInfoHolder(int count);
    void initialize();
    void copy(const CourseInfoHolder* pOther);
    bool readFromStream(sead::ReadStream* pStream);
    void writeToStream(sead::WriteStream* pStream, bool isSkip) const;

    /**
     * @brief Access the record of a course.
     * @param courseId Course identifier; the table has one entry per course plus entry 0.
     * @return The course record.
     */
    CourseInfo* getCourseInfo(int courseId) const { return &mpCourses[courseId]; }

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
bool isClearCharacter(GameDataHolderAccessor accessor, int courseId, int characterType);
bool isClearFlagTopCourseInfoHolder(const CourseInfoHolder* pHolder,
                                    GameDataHolderAccessor accessor, int courseId);
bool isClearCompleteCourseInfoHolder(const CourseInfoHolder* pHolder,
                                     GameDataHolderAccessor accessor, int courseId);
bool isAcquireIllustItemCourseInfoHolder(const CourseInfoHolder* pHolder,
                                         GameDataHolderAccessor accessor, int courseId);
} // namespace CourseInfoFunction

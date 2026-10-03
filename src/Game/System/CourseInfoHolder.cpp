#include "System/CourseInfoHolder.hpp"

/**
 * @brief Allocate and reset progress records for the course database.
 * @param count Nonnegative number of course records.
 */
CourseInfoHolder::CourseInfoHolder(int count) : mCount(count), mpCourses(nullptr) {
    mpCourses = new CourseInfo[count];
    initialize();
}

/**
 * @brief Reset progress for all courses.
 */
void CourseInfoHolder::initialize() {
    for (int i = 0; i < mCount; ++i) {
        mpCourses[i].reset();
    }
}

/**
 * @brief Copy all course progress records.
 * @param pOther Non-null source holder with at least mCount records.
 */
void CourseInfoHolder::copy(const CourseInfoHolder* pOther) {
    for (int i = 0; i < mCount; ++i) {
        mpCourses[i] = pOther->mpCourses[i];
    }
}

/**
 * @brief Read or update course progression.
 * @param writer Accessor to the active game-data holder.
 * @param courseId Valid course identifier in the current save file.
 * @return True when the requested course condition holds.
 */
bool CourseInfoFunction::setOpen(GameDataHolderWriter writer, int courseId) {
    return writer.getHolder()->getCourseInfoPtr(courseId)->setOpen();
}

/**
 * @brief Read or update course progression.
 * @param writer Accessor to the active game-data holder.
 * @param courseId Valid course identifier in the current save file.
 * @return True when the requested course condition holds.
 */
bool CourseInfoFunction::setGreenStarLock(GameDataHolderWriter writer, int courseId) {
    return writer.getHolder()->getCourseInfoPtr(courseId)->setGreenStarLock();
}

/**
 * @brief Read or update course progression.
 * @param writer Accessor to the active game-data holder.
 * @param courseId Valid course identifier in the current save file.
 */
void CourseInfoFunction::resetClearFlag(GameDataHolderWriter writer, int courseId) {
    writer.getHolder()->getCourseInfoPtr(courseId)->resetClearFlag();
}

/**
 * @brief Read or update course progression.
 * @param writer Accessor to the active game-data holder.
 * @param courseId Valid course identifier in the current save file.
 */
void CourseInfoFunction::openCasinoRoom(GameDataHolderWriter writer, int courseId) {
    writer.getHolder()->getCourseInfoPtr(courseId)->openCasinoRoom();
}

/**
 * @brief Read or update course progression.
 * @param writer Accessor to the active game-data holder.
 * @param courseId Valid course identifier in the current save file.
 */
void CourseInfoFunction::openGoldenExpress(GameDataHolderWriter writer, int courseId) {
    writer.getHolder()->getCourseInfoPtr(courseId)->openGoldenExpress();
}

/**
 * @brief Read or update course progression.
 * @param accessor Accessor to the active game-data holder.
 * @param courseId Valid course identifier in the current save file.
 * @return True when the requested course condition holds.
 */
bool CourseInfoFunction::isClose(GameDataHolderAccessor accessor, int courseId) {
    return accessor.getHolder()->getCourseInfo(courseId)->isClose();
}

/**
 * @brief Read or update course progression.
 * @param accessor Accessor to the active game-data holder.
 * @param courseId Valid course identifier in the current save file.
 * @return True when the requested course condition holds.
 */
bool CourseInfoFunction::isOpen(GameDataHolderAccessor accessor, int courseId) {
    return accessor.getHolder()->getCourseInfo(courseId)->isOpen();
}

/**
 * @brief Read or update course progression.
 * @param accessor Accessor to the active game-data holder.
 * @param courseId Valid course identifier in the current save file.
 * @return True when the requested course condition holds.
 */
bool CourseInfoFunction::isClear(GameDataHolderAccessor accessor, int courseId) {
    if (accessor.getHolder()->isSingleMode()) {
        return false;
    }
    return accessor.getHolder()->getCourseInfo(courseId)->isClear();
}

/**
 * @brief Read or update course progression.
 * @param accessor Accessor to the active game-data holder.
 * @param courseId Valid course identifier in the current save file.
 * @return True when the requested course condition holds.
 */
bool CourseInfoFunction::isGreenStarLock(GameDataHolderAccessor accessor, int courseId) {
    return accessor.getHolder()->getCourseInfo(courseId)->isGreenStarLock();
}

/**
 * @brief Read or update course progression.
 * @param writer Accessor to the active game-data holder.
 * @param courseId Valid course identifier in the current save file.
 */
void CourseInfoFunction::addMissCount(GameDataHolderWriter writer, int courseId) {
    writer.getHolder()->getCourseInfoPtr(courseId)->addMissCount();
}

/**
 * @brief Read or update course progression.
 * @param accessor Accessor to the active game-data holder.
 * @param courseId Valid course identifier in the current save file.
 * @return True when the requested course condition holds.
 */
bool CourseInfoFunction::isClearWithAssistBlock(GameDataHolderAccessor accessor, int courseId) {
    return accessor.getHolder()->getCourseInfo(courseId)->isClearWithAssistBlock();
}

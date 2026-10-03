#include "System/GameDataFunction.hpp"
#include "System/Data/StageListHolder.hpp"

/**
 * @brief Return the sentinel course identifier.
 * @return Zero, which does not identify a course.
 */
int GameDataFunction::getInvalidCourseId() { return 0; }

/**
 * @brief Check whether Bowser's Fury mode is active.
 * @param accessor Accessor to a valid game-data holder.
 * @return True for single mode.
 */
bool GameDataFunction::isSingleMode(GameDataHolderAccessor accessor) {
    return accessor.getHolder()->isSingleMode();
}

/**
 * @brief Read the stage database size.
 * @param accessor Accessor to a valid game-data holder.
 * @return The number of database records.
 */
int GameDataFunction::getWorldNum(GameDataHolderAccessor accessor) {
    return accessor.getHolder()->getStageList()->getWorldNum();
}

/**
 * @brief Read the stage database size.
 * @param accessor Accessor to a valid game-data holder.
 * @return The number of database records.
 */
int GameDataFunction::getCourseTotalNum(GameDataHolderAccessor accessor) {
    return accessor.getHolder()->getStageList()->getCourseTotalNum();
}

/**
 * @brief Find a stage by its unique course identifier.
 * @param accessor Accessor to a valid game-data holder.
 * @param courseId Course identifier to find.
 * @return The stage record, or nullptr when no course matches.
 */
StageDatabaseInfo* GameDataFunction::findStageDatabaseInfo(GameDataHolderAccessor accessor, int courseId) {
    return accessor.getHolder()->getStageList()->findStageDatabaseInfo(courseId);
}

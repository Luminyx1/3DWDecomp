#include "System/Data/StageListHolder.hpp"

/**
 * @brief Read the size of the stage database.
 * @return The number of records.
 */
int StageListHolder::getWorldNum() const { return mpWorldInfo->mWorldCount; }

/**
 * @brief Read the size of the stage database.
 * @return The number of records.
 */
int StageListHolder::getCourseTotalNum() const { return mpWorldInfo->mStageCount; }

/**
 * @brief Look up a world or stage database value.
 * @param courseId Valid identifier in the loaded stage database.
 * @return The requested record or database value.
 */
StageDatabaseInfo* StageListHolder::findStageDatabaseInfo(int courseId) const {
    return mpWorldInfo->findStageDatabaseInfo(courseId);
}

/**
 * @brief Look up a world or stage database value.
 * @param worldId Valid identifier in the loaded stage database.
 * @return The requested record or database value.
 */
WorldInfo* StageListHolder::findWorldInfo(int worldId) const { return mpWorldInfo->findWorldInfo(worldId); }

/**
 * @brief Look up a world or stage database value.
 * @param courseId Valid identifier in the loaded stage database.
 * @return The requested record or database value.
 */
const char* StageListHolder::findStageName(int courseId) const {
    return mpWorldInfo->findStageDatabaseInfo(courseId)->getStageName();
}

/**
 * @brief Look up a world or stage database value.
 * @param courseId Valid identifier in the loaded stage database.
 * @return The requested record or database value.
 */
const char* StageListHolder::findStageType(int courseId) const {
    return mpWorldInfo->findStageDatabaseInfo(courseId)->getTypeName();
}

/**
 * @brief Look up a world or stage database value.
 * @param courseId Valid identifier in the loaded stage database.
 * @return The requested record or database value.
 */
int StageListHolder::findStageTypeID(int courseId) const {
    return mpWorldInfo->findStageDatabaseInfo(courseId)->getTypeId();
}

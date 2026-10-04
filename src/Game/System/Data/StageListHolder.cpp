#include "System/Data/StageListHolder.hpp"
#include "System/GameDataFunction.hpp"
#include "al/Library/Resource/ResourceFunction.hpp"

/**
 * @brief Load the stage list resource and cache the special course identifiers of world 8.
 */
StageListHolder::StageListHolder() {
    al::Resource* pResource = al::findOrCreateResource("SystemData/StageList", nullptr);
    mpWorldInfo = StageInfoFunction::createWorldInfoList(pResource);
    mKoopaCastleCourseId = findKoopaCastleCourseId(8);
    mSpecialCourse100 = calcCourseId(8, 100);
    mSpecialCourse101 = calcCourseId(8, 101);
}

/**
 * @brief Find the Bowser castle course of a world.
 * @param worldId World to search.
 * @return The course identifier of the last Bowser castle stage in the world, or 0 if there is none.
 */
int StageListHolder::findKoopaCastleCourseId(int worldId) const {
    WorldInfo* pWorld = mpWorldInfo->findWorldInfo(worldId);
    int courseId = 0;
    for (int i = 0; i < pWorld->mStageCount; i++) {
        StageDatabaseInfo* pStage = pWorld->getStageInfoByIndex(i);
        if (pStage->isKoopaCastle()) {
            courseId = pStage->getCourseId();
        }
    }

    return courseId;
}

/**
 * @brief Convert a world and stage number to a course identifier.
 * @param worldId World to search.
 * @param stageId Stage number inside the world.
 * @return The matching course identifier, or the invalid course identifier if none exists.
 */
int StageListHolder::calcCourseId(int worldId, int stageId) const {
    WorldInfo* pWorld = mpWorldInfo->findWorldInfo(worldId);
    for (int i = 0; i < pWorld->mStageCount; i++) {
        StageDatabaseInfo* pStage = pWorld->getStageInfoByIndex(i);
        if (pStage->getStageId() == stageId) {
            return pStage->getCourseId();
        }
    }

    return GameDataFunction::getInvalidCourseId();
}

/**
 * @brief Read the number of worlds in the stage database.
 * @return The number of worlds.
 */
int StageListHolder::getWorldNum() const {
    return mpWorldInfo->mWorldCount;
}

/**
 * @brief Read the number of courses in the stage database.
 * @return The number of courses.
 */
int StageListHolder::getCourseTotalNum() const {
    return mpWorldInfo->mStageCount;
}

/**
 * @brief Convert a world and stage number to a course identifier.
 * @param worldId World to search.
 * @param stageId Stage number inside the world.
 * @return The matching course identifier, or the invalid course identifier if none exists.
 */
int StageListHolder::tryCalcCourseId(int worldId, int stageId) const {
    WorldInfo* pWorld = mpWorldInfo->findWorldInfo(worldId);
    for (int i = 0; i < pWorld->mStageCount; i++) {
        StageDatabaseInfo* pStage = pWorld->getStageInfoByIndex(i);
        if (pStage->getStageId() == stageId) {
            return pStage->getCourseId();
        }
    }

    return GameDataFunction::getInvalidCourseId();
}

/**
 * @brief Convert a course identifier to its world and stage number.
 * @param pWorldId Receives the world identifier, or 0 if the course is unknown.
 * @param pStageId Receives the stage number, or 0 if the course is unknown.
 * @param courseId Course to look up.
 * @return Whether the course was found.
 */
bool StageListHolder::tryCalcWorldAndStageId(int* pWorldId, int* pStageId, int courseId) const {
    const StageDatabaseInfo* pStage = mpWorldInfo->mpStages;
    for (int i = 0; i < mpWorldInfo->mStageCount; i++, pStage++) {
        if (pStage->getCourseId() == courseId) {
            *pWorldId = pStage->getWorldId();
            *pStageId = pStage->getStageId();
            return true;
        }
    }

    *pWorldId = 0;
    *pStageId = 0;
    return false;
}

/**
 * @brief Convert a course identifier to its world and stage number.
 * @param pWorldId Receives the world identifier, or 0 if the course is unknown.
 * @param pStageId Receives the stage number, or 0 if the course is unknown.
 * @param courseId Course to look up.
 */
void StageListHolder::calcWorldAndStageId(int* pWorldId, int* pStageId, int courseId) const {
    tryCalcWorldAndStageId(pWorldId, pStageId, courseId);
}

/**
 * @brief Look up the resource name of a course.
 * @param courseId Valid course identifier.
 * @return The stage resource name.
 */
const char* StageListHolder::findStageName(int courseId) const {
    return mpWorldInfo->findStageDatabaseInfo(courseId)->getStageName();
}

/**
 * @brief Look up the database record of a course.
 * @param courseId Valid course identifier.
 * @return The stage database record.
 */
StageDatabaseInfo* StageListHolder::findStageDatabaseInfo(int courseId) const {
    return mpWorldInfo->findStageDatabaseInfo(courseId);
}

/**
 * @brief Look up the textual stage type of a course.
 * @param courseId Valid course identifier.
 * @return The stage-type name.
 */
const char* StageListHolder::findStageType(int courseId) const {
    return mpWorldInfo->findStageDatabaseInfo(courseId)->getTypeName();
}

/**
 * @brief Look up the parsed stage type of a course.
 * @param courseId Valid course identifier.
 * @return The stage-type identifier.
 */
int StageListHolder::findStageTypeID(int courseId) const {
    return mpWorldInfo->findStageDatabaseInfo(courseId)->getTypeId();
}

/**
 * @brief Look up a world record.
 * @param worldId Valid world identifier.
 * @return The world record.
 */
WorldInfo* StageListHolder::findWorldInfo(int worldId) const {
    return mpWorldInfo->findWorldInfo(worldId);
}

#include "System/CourseInfoHolder.hpp"
#include "System/Data/StageDatabaseInfo.hpp"
#include "System/GameDataFile.hpp"
#include "System/GameDataFunction.hpp"
#include <stream/seadStream.h>

namespace {
/**
 * @brief Layout of one course record in a save stream.
 */
struct CourseInfoStreamData {
    CourseInfo mInfo;
    s32 mReserved = 0;
};

static_assert(sizeof(CourseInfoStreamData) == 0x20);

/**
 * @brief Number of 3D World save files checked when looking for a stamp in any file.
 */
constexpr s32 cGameDataFileNum = 4;

/**
 * @brief Access the course records of the active 3D World save file.
 * @param accessor Accessor to the active game-data holder.
 * @return The course records of the playing file.
 */
inline const CourseInfoHolder* getPlayingCourseInfoHolder(GameDataHolderAccessor accessor) {
    return accessor.getHolder()->getPlayingFile()->getCourseInfoHolder();
}

/**
 * @brief Access the course records of a save file that is in use.
 * @param accessor Accessor to the active game-data holder.
 * @param fileId Valid 3D World save-file index.
 * @return The file's course records, or nullptr for an unused file.
 */
inline const CourseInfoHolder* tryGetCourseInfoHolder(GameDataHolderAccessor accessor, int fileId) {
    if (accessor.getHolder()->getGameDataFile(fileId)->isNewFile()) {
        return nullptr;
    }

    return accessor.getHolder()->getGameDataFile(fileId)->getCourseInfoHolder();
}
} // namespace

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
 * @brief Load all course records from a save stream.
 * @param pStream Non-null stream positioned at the course records.
 * @return Always true.
 */
bool CourseInfoHolder::readFromStream(sead::ReadStream* pStream) {
    s32 count;
    pStream->readS32(count);
    CourseInfoStreamData data;
    for (s32 i = 0; i < count; ++i) {
        pStream->readMemBlock(&data, sizeof(CourseInfoStreamData));
        mpCourses[i] = data.mInfo;
    }

    return true;
}

/**
 * @brief Store all course records into a save stream.
 * @param pStream Non-null destination stream.
 * @param isSkip True to only advance the stream past the records.
 */
void CourseInfoHolder::writeToStream(sead::WriteStream* pStream, bool isSkip) const {
    pStream->writeS32(mCount);
    if (isSkip) {
        for (s32 i = 0; i < mCount; ++i) {
            pStream->skip(sizeof(CourseInfoStreamData));
        }

        return;
    }

    CourseInfoStreamData data;
    for (s32 i = 0; i < mCount; ++i) {
        data.mInfo = mpCourses[i];
        pStream->writeMemBlock(&data, sizeof(CourseInfoStreamData));
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
 * @brief Check whether a character has cleared a course.
 * @param accessor Accessor to the active game-data holder.
 * @param courseId Valid course identifier in the current save file.
 * @param characterType Playable character identifier.
 * @return True when the character has cleared the course.
 */
bool CourseInfoFunction::isClearCharacter(GameDataHolderAccessor accessor, int courseId,
                                          int characterType) {
    return accessor.getHolder()->getCourseInfo(courseId)->isClearCharacter(characterType);
}

/**
 * @brief Check whether a course is cleared at the top of its goal pole.
 * @param accessor Accessor to the active game-data holder.
 * @param courseId Valid course identifier in the current save file.
 * @return True for a top clear, or for any clear in a course without a goal pole.
 */
bool CourseInfoFunction::isClearFlagTop(GameDataHolderAccessor accessor, int courseId) {
    return isClearFlagTopCourseInfoHolder(getPlayingCourseInfoHolder(accessor), accessor, courseId);
}

/**
 * @brief Check whether a course is cleared at the top of its goal pole.
 * @param pHolder Non-null course records to examine.
 * @param accessor Accessor to the active game-data holder.
 * @param courseId Valid course identifier.
 * @return True for a top clear, or for any clear in a course without a goal pole.
 */
bool CourseInfoFunction::isClearFlagTopCourseInfoHolder(const CourseInfoHolder* pHolder,
                                                        GameDataHolderAccessor accessor,
                                                        int courseId) {
    const CourseInfo* pInfo = pHolder->getCourseInfo(courseId);
    if (GameDataFunction::findStageDatabaseInfo(accessor, courseId)->isUseGoalPole()) {
        return pInfo->getClearValue() >= 1.0f;
    }

    return pInfo->isClear();
}

/**
 * @brief Check whether everything in a course of the playing file has been collected.
 * @param accessor Accessor to the active game-data holder.
 * @param courseId Valid course identifier in the current save file.
 * @return True when the course is fully completed.
 */
bool CourseInfoFunction::isClearComplete(GameDataHolderAccessor accessor, int courseId) {
    return isClearCompleteCourseInfoHolder(getPlayingCourseInfoHolder(accessor), accessor,
                                           courseId);
}

/**
 * @brief Check whether everything in a course has been collected.
 * @param pHolder Non-null course records to examine.
 * @param accessor Accessor to the active game-data holder.
 * @param courseId Valid course identifier.
 * @return True for a top clear with all Green Stars, the stamp and every character's clear;
 *         Captain Toad courses only need to be cleared.
 */
bool CourseInfoFunction::isClearCompleteCourseInfoHolder(const CourseInfoHolder* pHolder,
                                                         GameDataHolderAccessor accessor,
                                                         int courseId) {
    const CourseInfo* pInfo = pHolder->getCourseInfo(courseId);
    const StageDatabaseInfo* pStage = GameDataFunction::findStageDatabaseInfo(accessor, courseId);
    if (pStage->isKinopioBrigade()) {
        return pInfo->isClear();
    }

    if (!isClearFlagTopCourseInfoHolder(pHolder, accessor, courseId)) {
        return false;
    }

    if (!pInfo->getGreenStarInfo().isCompleteAcquire(pStage->getGreenStarNum())) {
        return false;
    }

    if (pStage->getIllustItemNum() != 0 && !pInfo->isAcquireIllustItem()) {
        return false;
    }

    return pInfo->isClearAllCharacter();
}

/**
 * @brief Read the character that recorded the top clear of a course.
 * @param accessor Accessor to the active game-data holder.
 * @param courseId Valid course identifier in the current save file.
 * @return The stored clear type.
 */
s32 CourseInfoFunction::getClearTopCharacter(GameDataHolderAccessor accessor, int courseId) {
    return accessor.getHolder()->getCourseInfo(courseId)->getClearType();
}

/**
 * @brief Access the Green Star flags of a course.
 * @param accessor Accessor to the active game-data holder.
 * @param courseId Valid course identifier in the current save file.
 * @return The course's Green Star record.
 */
const CourseGreenStarInfo* CourseInfoFunction::getGreenStarAcquireFlag(
    GameDataHolderAccessor accessor, int courseId) {
    return &accessor.getHolder()->getCourseInfo(courseId)->getGreenStarInfo();
}

/**
 * @brief Read the highest goal-pole position reached in a course.
 * @param accessor Accessor to the active game-data holder.
 * @param courseId Valid course identifier in the current save file.
 * @return The stored clear value.
 */
f32 CourseInfoFunction::getHighestGoalPolePosition(GameDataHolderAccessor accessor,
                                                   int courseId) {
    return accessor.getHolder()->getCourseInfo(courseId)->getClearValue();
}

/**
 * @brief Read the best score of a course.
 * @param accessor Accessor to the active game-data holder.
 * @param courseId Valid course identifier in the current save file.
 * @return The best score, or -1 when none is recorded.
 */
s32 CourseInfoFunction::getBestScore(GameDataHolderAccessor accessor, int courseId) {
    return accessor.getHolder()->getCourseInfo(courseId)->getBestScore();
}

/**
 * @brief Read the best clear time of a course.
 * @param accessor Accessor to the active game-data holder.
 * @param courseId Valid course identifier in the current save file.
 * @return The best time, or -1 when none is recorded.
 */
s32 CourseInfoFunction::getBestTime(GameDataHolderAccessor accessor, int courseId) {
    return accessor.getHolder()->getCourseInfo(courseId)->getBestTime();
}

/**
 * @brief Check whether every Green Star of a course has been collected.
 * @param accessor Accessor to the active game-data holder.
 * @param courseId Valid course identifier in the current save file.
 * @return True when all of the course's Green Stars are acquired.
 */
bool CourseInfoFunction::isAcquireGreenStarAll(GameDataHolderAccessor accessor, int courseId) {
    const StageDatabaseInfo* pStage = GameDataFunction::findStageDatabaseInfo(accessor, courseId);
    const CourseInfo* pInfo = accessor.getHolder()->getCourseInfo(courseId);
    return pInfo->getGreenStarInfo().isCompleteAcquire(pStage->getGreenStarNum());
}

/**
 * @brief Check whether the stamp of a course has been collected.
 * @param accessor Accessor to the active game-data holder.
 * @param courseId Valid course identifier.
 * @param isAllFile False to check the playing file only; true to check save files.
 * @param fileId Save file to check when isAllFile is set, or a negative value for any file.
 * @return True when the stamp has been collected.
 */
bool CourseInfoFunction::isAcquireIllustItem(GameDataHolderAccessor accessor, int courseId,
                                             bool isAllFile, int fileId) {
    if (!isAllFile) {
        return isAcquireIllustItemCourseInfoHolder(getPlayingCourseInfoHolder(accessor), accessor,
                                                   courseId);
    }

    if (fileId >= 0) {
        const CourseInfoHolder* pHolder = tryGetCourseInfoHolder(accessor, fileId);
        if (pHolder == nullptr) {
            return false;
        }

        if (isAcquireIllustItemCourseInfoHolder(pHolder, accessor, courseId)) {
            return true;
        }
    } else {
        for (s32 i = 0; i < cGameDataFileNum; ++i) {
            const CourseInfoHolder* pHolder = tryGetCourseInfoHolder(accessor, i);
            if (pHolder != nullptr &&
                isAcquireIllustItemCourseInfoHolder(pHolder, accessor, courseId)) {
                return true;
            }
        }
    }

    return false;
}

/**
 * @brief Check whether the stamp of a course has been collected.
 * @param pHolder Non-null course records to examine.
 * @param accessor Unused accessor to the active game-data holder.
 * @param courseId Valid course identifier.
 * @return True when the stamp has been collected.
 */
bool CourseInfoFunction::isAcquireIllustItemCourseInfoHolder(const CourseInfoHolder* pHolder,
                                                             GameDataHolderAccessor accessor,
                                                             int courseId) {
    return pHolder->getCourseInfo(courseId)->isAcquireIllustItem();
}

/**
 * @brief Read how many times the player missed in a course.
 * @param accessor Accessor to the active game-data holder.
 * @param courseId Valid course identifier in the current save file.
 * @return The miss count.
 */
s32 CourseInfoFunction::getMissCount(GameDataHolderAccessor accessor, int courseId) {
    return accessor.getHolder()->getCourseInfo(courseId)->getMissCount();
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

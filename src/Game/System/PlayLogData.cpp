#include "System/PlayLogData.hpp"
#include <stream/seadStream.h>
#include <cstring>

/**
 * @brief Clear the common and per-course play-log values.
 */
void PlayLogData::initializeData() { std::memset(mpValues, 0, 4000); }

/**
 * @brief Access the common play-log counters.
 * @return The start of the common counter block.
 */
u32* PlayLogData::getCommonData() { return mpValues; }

/**
 * @brief Find the play-log counters for a course.
 * @param courseId Course-table index; must be within the allocated course count.
 * @return The course counter block, or nullptr for a course without play-log data.
 */
u32* PlayLogData::tryGetCourseData(int courseId) {
    const s32 offset = mpCourseOffsets[courseId];
    if (offset < 0) {
        return nullptr;
    }
    return mpValues + offset;
}

/**
 * @brief Read the fixed-size play-log storage.
 * @param pStream Non-null input stream positioned at the play-log block.
 * @return Always true; stream errors are handled by the stream.
 */
bool PlayLogData::readFromStream(sead::ReadStream* pStream) {
    pStream->readMemBlock(mpValues, 4000);
    return true;
}

/**
 * @brief Write the fixed-size play-log storage.
 * @param pStream Non-null output stream receiving 4000 bytes.
 */
void PlayLogData::writeToStream(sead::WriteStream* pStream) const { pStream->writeMemBlock(mpValues, 4000); }

#include "System/GameDataFunction.hpp"
#include "System/Data/StageDatabaseInfo.hpp"

/**
 * @brief Allocate common and per-course play-log counters.
 * @param pHolder Non-null holder with a loaded course database that fits the 1000-counter buffer.
 */
PlayLogData::PlayLogData(GameDataHolder* pHolder)
    : mpHolder(pHolder), mUsedValueCount(0), mpCourseOffsets(nullptr), mCourseCount(0) {
    mpValues = new u32[1000];
    std::memset(mpValues, 0, 4000);
    mCourseCount = GameDataFunction::getCourseTotalNum(GameDataHolderAccessor(pHolder)) + 1;
    mpCourseOffsets = new s32[mCourseCount];
    mpCourseOffsets[0] = -1;
    int offset = 39;
    for (int i = 1; i < mCourseCount; ++i) {
        const auto* pStage = GameDataFunction::findStageDatabaseInfo(GameDataHolderAccessor(pHolder), i);
        if (pStage->isCasinoRoom() || pStage->isKinopioHouse() || pStage->isFairyHouse() ||
            pStage->isKinopioHouseHide() || pStage->isDokanHide()) {
            mpCourseOffsets[i] = -1;
        } else {
            mpCourseOffsets[i] = offset;
            offset += 8;
        }
    }
    mUsedValueCount = offset;
}

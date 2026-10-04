#include "System/CourseInfo.hpp"
#include "System/CourseInfoHolder.hpp"
#include <stream/seadStream.h>

namespace {
/**
 * @brief Converts a playable character identifier into its course-clear flag.
 * @param characterType Character identifier from 0 to 4; other values have no flag.
 * @return The character's flag, or zero for an unsupported identifier.
 */
inline u32 getCharacterClearFlag(s32 characterType) {
    switch (characterType) {
    case 0:
        return 8;
    case 1:
        return 16;
    case 2:
        return 32;
    case 3:
        return 64;
    case 4:
        return 128;
    default:
        return 0;
    }
}
} // namespace

/**
 * @brief Creates an empty course record with unset score and time records.
 */
CourseInfo::CourseInfo() { reset(); }

/**
 * @brief Resets course flags, clear information, records, stars, and the miss count.
 */
void CourseInfo::reset() {
    mFlags = 0;
    mClearType = 0;
    mClearValue = 0.0f;
    mBestScore = -1;
    mBestTime = -1;
    mGreenStars.initialize();
    mMissCount = 0;
}

/**
 * @brief Clears all course progress flags.
 */
void CourseInfo::resetClearFlag() { mFlags = 0; }

/**
 * @brief Resets the casino room flags to the open state.
 */
void CourseInfo::openCasinoRoom() { mFlags = Open; }

/**
 * @brief Resets the Golden Express flags to the open state.
 */
void CourseInfo::openGoldenExpress() { mFlags = Open; }

/**
 * @brief Reopens a cleared Toad House and discards its other progress flags.
 */
void CourseInfo::resetKinopioHouse() {
    if (isClear()) {
        mFlags = Open;
    }
}

/**
 * @brief Checks the ordinary course-clear flag.
 * @return True if the course has been cleared.
 */
bool CourseInfo::isClear() const { return hasAnyFlag(Clear); }

/**
 * @brief Sets the open flag if it is not already set.
 * @return True if this call newly sets the open flag.
 */
bool CourseInfo::setOpen() {
    if (isOpen()) {
        return false;
    }
    mFlags |= Open;
    return true;
}

/**
 * @brief Checks whether the course is open.
 * @return True when the open flag is set.
 */
bool CourseInfo::isOpen() const { return hasAnyFlag(Open); }

/**
 * @brief Marks an uncleared course as cleared and removes its warp-clear flag.
 * @return True if the ordinary clear flag was newly set.
 */
bool CourseInfo::setClear() {
    if (isClear()) {
        return false;
    }
    mFlags &= ~WorldWarpClear;
    mFlags |= Clear;
    return true;
}

/**
 * @brief Sets the warp-clear flag unless the course is already ordinarily cleared.
 * @return True when the course does not have the ordinary clear flag.
 */
bool CourseInfo::setWorldWarpClear() {
    if (isClear()) {
        return false;
    }
    mFlags |= WorldWarpClear;
    return true;
}

/**
 * @brief Sets the Green Star lock flag unless the course is already exclusively locked.
 * @return False only if the existing low three flags already represent the locked state.
 */
bool CourseInfo::setGreenStarLock() {
    if (isGreenStarLock()) {
        return false;
    }
    mFlags |= GreenStarLock;
    return true;
}

/**
 * @brief Checks for a Green Star lock without open or clear flags.
 * @return True when locked and neither open nor cleared.
 */
bool CourseInfo::isGreenStarLock() const {
    return (mFlags & (Open | Clear | GreenStarLock)) == GreenStarLock;
}

/**
 * @brief Checks whether the course has no open, clear, or Green Star lock flag.
 * @return True when all three state flags are clear.
 */
bool CourseInfo::isClose() const { return !hasAnyFlag(Open | Clear | GreenStarLock); }

/**
 * @brief Checks the warp-clear flag.
 * @return True when the warp-clear flag is set.
 */
bool CourseInfo::isWorldWarpClear() const { return hasAnyFlag(WorldWarpClear); }

/**
 * @brief Records a clear with the specified playable character.
 * @param characterType Character identifier from 0 to 4; other values leave the flags unchanged.
 */
void CourseInfo::setClearCharacter(s32 characterType) { mFlags |= getCharacterClearFlag(characterType); }

/**
 * @brief Checks whether the course was cleared with a specific character.
 * @param characterType Character identifier from 0 to 4; other values return false.
 * @return True when that character has a clear flag.
 */
bool CourseInfo::isClearCharacter(s32 characterType) const {
    const u32 flags = mFlags;
    return (flags & getCharacterClearFlag(characterType)) != 0;
}

/**
 * @brief Checks whether all five playable characters have cleared the course.
 * @return True when all five character flags are set.
 */
bool CourseInfo::isClearAllCharacter() const { return hasAllFlags(AllCharacters); }

/**
 * @brief Updates the acquired stamp flag.
 * @param acquired Whether the course stamp has been acquired.
 */
void CourseInfo::setAcquireIllustItem(bool acquired) {
    if (acquired) {
        mFlags |= IllustItem;
    } else {
        mFlags &= ~IllustItem;
    }
}

/**
 * @brief Checks whether the course stamp has been acquired.
 * @return True when the stamp flag is set.
 */
bool CourseInfo::isAcquireIllustItem() const { return hasAnyFlag(IllustItem); }

/**
 * @brief Counts acquired Green Stars in the requested range.
 * @param starNum Number of leading star flags, from 0 to 32; negative values count all 32.
 * @return Number of acquired stars in the range.
 */
s32 CourseInfo::calcGreenStarAcquireNum(s32 starNum) const {
    return mGreenStars.calcGreenStarAcquireNum(starNum);
}

/**
 * @brief Checks a single acquired Green Star flag.
 * @param starIndex Zero-based Green Star bit index, from 0 to 31.
 * @return True when the selected star was acquired.
 */
bool CourseInfo::isGreenStarAcquire(s32 starIndex) const { return mGreenStars.isAcquired(starIndex); }

/**
 * @brief Increments the miss count, saturating at 255.
 */
void CourseInfo::addMissCount() {
    const s32 next = static_cast<s32>(static_cast<u32>(mMissCount) + 1);
    mMissCount = next < 255 ? next : 255;
}

/**
 * @brief Checks whether both the clear and assist-block flags are set.
 * @return True when the course has an assisted clear.
 */
bool CourseInfo::isClearWithAssistBlock() const { return hasAllFlags(Clear | AssistBlock); }

/**
 * @brief Sets the assist-block flag without changing the ordinary clear flag.
 */
void CourseInfo::setClearWithAssistBlock() { mFlags |= AssistBlock; }

/**
 * @brief Clears the assist-block flag.
 */
void CourseInfo::resetClearWithAssistBlock() { mFlags &= ~AssistBlock; }

/**
 * @brief Replaces clear information when the new value is at least the recorded value.
 * @param clearType Clear classification associated with the new value.
 * @param clearValue Comparable clear result; lower values leave the current result unchanged.
 */
void CourseInfo::setClearInfo(s32 clearType, f32 clearValue) {
    if (mClearValue <= clearValue) {
        mClearValue = clearValue;
        mClearType = clearType;
    }
}

/**
 * @brief Copies the acquired Green Star flags into this course.
 * @param rStars Source set of acquired Green Stars.
 */
void CourseInfo::setGreenStarAcquireFlag(const CourseGreenStarInfo& rStars) { mGreenStars.copy(rStars); }

/**
 * @brief Updates the best score when the new nonnegative score improves the record.
 * @param score Candidate score; negative values are ignored.
 * @return True if an existing positive record was beaten; initializing a record returns false.
 */
bool CourseInfo::setBestScore(s32 score) {
    if (score < 0) {
        return false;
    }
    if (mBestScore < 1 || mBestScore < score) {
        bool hadRecord = mBestScore > 0;
        mBestScore = score;
        return hadRecord;
    }
    return false;
}

/**
 * @brief Updates the best time when the new positive time improves the record.
 * @param time Candidate time in the game's stored units; nonpositive values are ignored.
 * @return True if a previously set record was beaten; initializing a record returns false.
 */
bool CourseInfo::setBestTime(s32 time) {
    if (time < 1) {
        return false;
    }
    if (mBestTime == -1 || mBestTime > time) {
        bool hadRecord = mBestTime != -1;
        mBestTime = time;
        return hadRecord;
    }
    return false;
}

namespace {
/**
 * @brief Layout of one course record in a save stream.
 */
struct CourseInfoStreamData {
    CourseInfo mInfo;
    s32 mReserved = 0;
};

static_assert(sizeof(CourseInfoStreamData) == 0x20);
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

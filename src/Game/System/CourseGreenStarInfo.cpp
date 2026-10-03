#include "System/CourseGreenStarInfo.hpp"

/**
 * @brief Creates an empty set of acquired Green Stars.
 */
CourseGreenStarInfo::CourseGreenStarInfo() : mFlags(0) {}

/**
 * @brief Clears all acquired Green Star flags.
 */
void CourseGreenStarInfo::initialize() { mFlags = 0; }

/**
 * @brief Copies the acquired flags from another course record.
 * @param rOther Record containing the flags to copy; self-copy is permitted.
 */
void CourseGreenStarInfo::copy(const CourseGreenStarInfo& rOther) { mFlags = rOther.mFlags; }

/**
 * @brief Counts acquired stars in the requested prefix of the flag set.
 * @param starNum Number of bits to examine, from 0 to 32; a negative value examines all 32.
 * @return Number of acquired stars in the requested range.
 */
s32 CourseGreenStarInfo::calcGreenStarAcquireNum(s32 starNum) const {
    if (starNum < 0) {
        starNum = 32;
    }
    s32 count = 0;
    for (s32 i = 0; i < starNum; ++i) {
        if (isAcquired(i)) {
            ++count;
        }
    }
    return count;
}

/**
 * @brief Checks whether every requested Green Star has been acquired.
 * @param starNum Number of leading flags to check, at most 32; nonpositive values return true.
 * @return True when all requested flags are set.
 */
bool CourseGreenStarInfo::isCompleteAcquire(s32 starNum) const {
    for (s32 i = 0; i < starNum; ++i) {
        if (!isAcquired(i)) {
            return false;
        }
    }
    return true;
}

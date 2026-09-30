#include "Project/Base/StringMatcher.hpp"

#include "Project/Base/StringOpUtil.hpp"

namespace al {
/**
 * Constructs a matcher for a wildcard pattern.
 * @param rMatch The pattern.
 */
StringMatcher::StringMatcher(const MatchStr& rMatch) : mMatchStr(rMatch.mStr) {}

/**
 * Matches a string against the pattern, recording the wildcard matches.
 * @param pStr The string.
 * @return True if the whole string matched.
 */
bool StringMatcher::tryMatch(const char* pStr) {
    const char* rest = nullptr;
    getSubStringUnmatched(&rest, pStr);
    return *rest == '\0';
}

/**
 * Matches a string against the pattern, recording the wildcard matches.
 * @param pOutRest Receives the unmatched rest of the string.
 * @param pStr The string.
 * @return The result of the match.
 */
const char* StringMatcher::getSubStringUnmatched(const char** pOutRest, const char* pStr) {
    resetMatchInfo();
    return al::getSubStringUnmatched(pOutRest, pStr, mMatchStr, &handleAddMatchInfo, this);
}

/**
 * Clears all recorded matches.
 */
void StringMatcher::resetMatchInfo() {
    for (s32 i = 0; i < 10; i++) {
        mMatchInfo[i].mStart = nullptr;
        mMatchInfo[i].mEnd = nullptr;
    }
}

/**
 * Match callback that records a match in the matcher passed as user data.
 * @param pStart Start of the match.
 * @param pEnd End of the match.
 * @param pUser The matcher.
 */
void StringMatcher::handleAddMatchInfo(const char* pStart, const char* pEnd, void* pUser) {
    static_cast<StringMatcher*>(pUser)->addMatchInfo(pStart, pEnd);
}

/**
 * Counts the recorded matches.
 * @return The number of matches.
 */
s32 StringMatcher::calcMatchNum() const {
    for (s32 i = 0; i < 10; i++) {
        if (!mMatchInfo[i].mStart) {
            return i;
        }
    }
    return 10;
}

/**
 * Copies a recorded match into a string.
 * @param pOut Output string.
 * @param index Match index.
 */
void StringMatcher::getMatchedString(sead::BufferedSafeString* pOut, s32 index) const {
    const MatchInfo& info = mMatchInfo[index];
    extractString(pOut->getBuffer(), info.mStart, info.mEnd - info.mStart, pOut->getBufferSize());
}

/**
 * Gets a recorded match.
 * @param index Match index.
 * @return The match.
 */
const StringMatcher::MatchInfo& StringMatcher::getMatchInfo(s32 index) const {
    return mMatchInfo[index];
}

/**
 * Records a match in the first free slot.
 * @param pStart Start of the match.
 * @param pEnd End of the match.
 */
void StringMatcher::addMatchInfo(const char* pStart, const char* pEnd) {
    s32 index = calcMatchNum();
    if (index >= 10) {
        return;
    }
    mMatchInfo[index].mStart = pStart;
    mMatchInfo[index].mEnd = pEnd;
}
}  // namespace al

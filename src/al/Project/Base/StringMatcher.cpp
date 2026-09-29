#include "Project/Base/StringMatcher.hpp"
#include "Project/Base/StringOpUtil.hpp"

namespace al {
    /**
     * @brief Constructs a matcher for a pattern.
     * @param rMatchStr The pattern to match against.
     */
    StringMatcher::StringMatcher(const MatchStr& rMatchStr) : mMatchStr(rMatchStr) {}

    /**
     * @brief Matches a string against the pattern, recording the matched parts.
     * @param pStr The string to match.
     * @return Whether the whole string matches the pattern.
     */
    bool StringMatcher::tryMatch(const char* pStr) {
        const char* unmatched = nullptr;
        getSubStringUnmatched(&unmatched, pStr);
        return *unmatched == '\0';
    }

    /**
     * @brief Matches a string against the pattern, recording the matched parts.
     * @param pUnmatched Receives the part of the string that was not matched.
     * @param pStr The string to match.
     * @return The part of the string that was not matched.
     */
    const char* StringMatcher::getSubStringUnmatched(const char** pUnmatched, const char* pStr) {
        resetMatchInfo();
        MatchStr matchStr = mMatchStr;
        return al::getSubStringUnmatched(pUnmatched, pStr, matchStr, handleAddMatchInfo, this);
    }

    /** @brief Clears all recorded matches. */
    void StringMatcher::resetMatchInfo() {
        for (s32 i = 0; i < 10; i++) {
            mMatchInfo[i].mStart = nullptr;
            mMatchInfo[i].mEnd = nullptr;
        }
    }

    /**
     * @brief Callback that records a matched part of a string.
     * @param pStart The start of the matched part.
     * @param pEnd The end of the matched part.
     * @param pMatcher The matcher to record the match in.
     */
    void StringMatcher::handleAddMatchInfo(const char* pStart, const char* pEnd, void* pMatcher) {
        static_cast<StringMatcher*>(pMatcher)->addMatchInfo(pStart, pEnd);
    }

    /**
     * @brief Counts the recorded matches.
     * @return The number of recorded matches.
     */
    s32 StringMatcher::calcMatchNum() const {
        for (s32 i = 0; i < 10; i++) {
            if (mMatchInfo[i].mStart == nullptr) {
                return i;
            }
        }

        return 10;
    }

    /**
     * @brief Copies a recorded match into a string.
     * @param pOut The string to copy the match into.
     * @param index The index of the match.
     */
    void StringMatcher::getMatchedString(sead::BufferedSafeString* pOut, s32 index) const {
        const MatchInfo& info = mMatchInfo[index];
        extractString(pOut->getBuffer(), info.mStart, info.mEnd - info.mStart, pOut->getBufferSize());
    }

    /**
     * @brief Gets a recorded match.
     * @param index The index of the match.
     * @return The match's info.
     */
    const StringMatcher::MatchInfo* StringMatcher::getMatchInfo(s32 index) const {
        return &mMatchInfo[index];
    }

    /**
     * @brief Records a matched part of a string in the first free slot.
     * @param pStart The start of the matched part.
     * @param pEnd The end of the matched part.
     */
    void StringMatcher::addMatchInfo(const char* pStart, const char* pEnd) {
        s32 index = calcMatchNum();
        if (index >= 10) {
            return;
        }

        mMatchInfo[index].mStart = pStart;
        mMatchInfo[index].mEnd = pEnd;
    }
};

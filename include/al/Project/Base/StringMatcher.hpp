#pragma once

#include <prim/seadSafeString.h>

#include "Project/Base/StringUtil.hpp"

namespace al {
class StringMatcher {
public:
    struct MatchInfo {
        const char* mStart;
        const char* mEnd;
    };

    StringMatcher(const MatchStr& rMatch);

    bool tryMatch(const char* pStr);
    const char* getSubStringUnmatched(const char** pOutRest, const char* pStr);
    void resetMatchInfo();
    static void handleAddMatchInfo(const char* pStart, const char* pEnd, void* pUser);
    s32 calcMatchNum() const;
    void getMatchedString(sead::BufferedSafeString* pOut, s32 index) const;
    const MatchInfo& getMatchInfo(s32 index) const;
    void addMatchInfo(const char* pStart, const char* pEnd);

    const char* mMatchStr;
    MatchInfo mMatchInfo[10];
};
}  // namespace al

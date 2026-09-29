#pragma once

#include <basis/seadTypes.h>
#include <prim/seadSafeString.h>
#include "Project/Base/StringUtil.hpp"

namespace al {
    /// Matches strings against a wildcard pattern and records the matched parts.
    class StringMatcher {
    public:
        struct MatchInfo {
            const char* mStart;     // _0
            const char* mEnd;       // _8
        };

        StringMatcher(const MatchStr& rMatchStr);

        bool tryMatch(const char* pStr);
        const char* getSubStringUnmatched(const char** pUnmatched, const char* pStr);
        void resetMatchInfo();
        static void handleAddMatchInfo(const char* pStart, const char* pEnd, void* pMatcher);
        s32 calcMatchNum() const;
        void getMatchedString(sead::BufferedSafeString* pOut, s32 index) const;
        const MatchInfo* getMatchInfo(s32 index) const;
        void addMatchInfo(const char* pStart, const char* pEnd);

        MatchStr mMatchStr;             // _0
        MatchInfo mMatchInfo[10];       // _8
    };
};

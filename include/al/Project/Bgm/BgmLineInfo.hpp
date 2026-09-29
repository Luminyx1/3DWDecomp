#pragma once

#include <basis/seadTypes.h>

#include "Project/Audio/AudioInfoList.hpp"

namespace al {
class ByamlIter;

struct BgmLineInfo {
    static BgmLineInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const BgmLineInfo* pInfoA, const BgmLineInfo* pInfoB);

    const char* mName;                                // _0
    s32 mPriority;                                    // _8
    bool mIsDontChangeLowPriorityLineByAreaChange;    // _C
    bool mIsDontStopByBgmStopArea;                    // _D
    bool mIsDontStopByChangeBgmArea;                  // _E
};

struct BgmCombinedLineInfo {
    static BgmCombinedLineInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const BgmCombinedLineInfo* pInfoA, const BgmCombinedLineInfo* pInfoB);

    const char* mName;                          // _0
    AudioInfoList<BgmLineInfo>* mLineInfoList;  // _8
};
}  // namespace al

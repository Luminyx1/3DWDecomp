#pragma once

#include <basis/seadTypes.h>

#include "Project/Audio/AudioInfoList.hpp"

namespace al {
class ByamlIter;

struct BgmStagePlayInfo {
    static BgmStagePlayInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const BgmStagePlayInfo* pInfoA, const BgmStagePlayInfo* pInfoB);

    const char* mName;                // _0
    const char* mResourceName;        // _8
    const char* mRegionInfoListName;  // _10
    s32 mStartDelayFrameNum;          // _18
    s32 mFadeInFrameNum;              // _1C
};

struct BgmStageInfo {
    static BgmStageInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const BgmStageInfo* pInfoA, const BgmStageInfo* pInfoB);

    const char* mName;                               // _0
    AudioInfoList<BgmStagePlayInfo>* mPlayInfoList;  // _8
};
}  // namespace al

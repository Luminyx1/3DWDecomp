#pragma once

#include <basis/seadTypes.h>

#include "Project/Audio/AudioInfoList.hpp"

namespace al {
class ByamlIter;

struct BgmResourceSuffixInfo {
    static BgmResourceSuffixInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const BgmResourceSuffixInfo* pInfoA, const BgmResourceSuffixInfo* pInfoB);

    const char* mName;  // _0
    s32 mStartSample;   // _8
    f32 mBpm;           // _C
    s32 mSampleRate;    // _10
};

struct BgmEnableSituationInfo {
    static BgmEnableSituationInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const BgmEnableSituationInfo* pInfoA, const BgmEnableSituationInfo* pInfoB);

    const char* mName;              // _0
    const char* mSubSituationName;  // _8
};

struct BgmStartTriggerSituationInfo {
    static BgmStartTriggerSituationInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const BgmStartTriggerSituationInfo* pInfoA,
                           const BgmStartTriggerSituationInfo* pInfoB);

    const char* mName;  // _0
};

struct BgmResourceInfo {
    static BgmResourceInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const BgmResourceInfo* pInfoA, const BgmResourceInfo* pInfoB);

    const char* mName;                                                     // _0
    s32 mStartSample;                                                      // _8
    f32 mBpm;                                                              // _C
    s32 mSampleRate;                                                       // _10
    AudioInfoList<BgmResourceSuffixInfo>* mSuffixInfoList;                 // _18
    AudioInfoList<BgmEnableSituationInfo>* mEnableSituationInfoList;       // _20
    AudioInfoList<BgmStartTriggerSituationInfo>* mStartTriggerSituationInfoList;  // _28
    bool mIsEnableRegionJump;                                              // _30
    bool mIsDisableAudioEffect;                                            // _31
    bool mIsEnableNwRender;                                                // _32
};
}  // namespace al

#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>
#include <prim/seadSafeString.h>

#include "Project/Audio/AudioInfoList.hpp"

namespace al {
class ByamlIter;
class BgmUserInfo;

class BgmLineInfo {
public:
    static BgmLineInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const BgmLineInfo* pA, const BgmLineInfo* pB);

    const char* mName;
    s32 mPriority;
    bool mIsDontChangeLowPriorityLineByAreaChange;
    bool mIsDontStopByBgmStopArea;
    bool mIsDontStopByChangeBgmArea;
};
static_assert(sizeof(BgmLineInfo) == 0x10);

class BgmCombinedLineInfo {
public:
    static BgmCombinedLineInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const BgmCombinedLineInfo* pA, const BgmCombinedLineInfo* pB);

    const char* mName;
    AudioInfoList<BgmLineInfo>* mLineInfoList;
};
static_assert(sizeof(BgmCombinedLineInfo) == 0x10);

class BgmPlayInfo {
public:
    static BgmPlayInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const BgmPlayInfo* pA, const BgmPlayInfo* pB);

    const char* mName;
    const char* mLineName;
    const char* mDefaultResourceName;
    bool mIsPlayingByUpperLayerAudioUser;
};
static_assert(sizeof(BgmPlayInfo) == 0x20);

class BgmResourceSuffixInfo {
public:
    static BgmResourceSuffixInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const BgmResourceSuffixInfo* pA, const BgmResourceSuffixInfo* pB);

    const char* mSuffixName;
    s32 mStartSample;
    f32 mBpm;
    s32 mSampleRate;
};
static_assert(sizeof(BgmResourceSuffixInfo) == 0x18);

class BgmEnableSituationInfo {
public:
    static BgmEnableSituationInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const BgmEnableSituationInfo* pA, const BgmEnableSituationInfo* pB);

    const char* mName;
    const char* mSubSituationName;
};
static_assert(sizeof(BgmEnableSituationInfo) == 0x10);

class BgmStartTriggerSituationInfo {
public:
    static BgmStartTriggerSituationInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const BgmStartTriggerSituationInfo* pA, const BgmStartTriggerSituationInfo* pB);

    const char* mName;
};
static_assert(sizeof(BgmStartTriggerSituationInfo) == 0x8);

class BgmResourceInfo {
public:
    static BgmResourceInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const BgmResourceInfo* pA, const BgmResourceInfo* pB);

    const char* mName;
    s32 mStartSample;
    f32 mBpm;
    s32 mSampleRate;
    AudioInfoList<BgmResourceSuffixInfo>* mResourceSuffixInfoList;
    AudioInfoList<BgmEnableSituationInfo>* mEnableSituationInfoList;
    AudioInfoList<BgmStartTriggerSituationInfo>* mStartTriggerSituationInfoList;
    bool mIsEnableRegionJump;
    bool mIsDisableAudioEffect;
    bool mIsEnableNwRender;
};
static_assert(sizeof(BgmResourceInfo) == 0x38);

class BgmStagePlayInfo {
public:
    static BgmStagePlayInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const BgmStagePlayInfo* pA, const BgmStagePlayInfo* pB);

    const char* mPlayInfoName;
    const char* mResourceName;
    const char* mRegionInfoListName;
    s32 mStartDelayFrameNum;
    s32 mFadeInFrameNum;
};
static_assert(sizeof(BgmStagePlayInfo) == 0x20);

class BgmStageInfo {
public:
    static BgmStageInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const BgmStageInfo* pA, const BgmStageInfo* pB);

    const char* mName;
    AudioInfoList<BgmStagePlayInfo>* mStagePlayInfoList;
};
static_assert(sizeof(BgmStageInfo) == 0x10);

class BgmProcInfo {
public:
    static BgmProcInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const BgmProcInfo* pA, const BgmProcInfo* pB);

    virtual void dummy() {}

    const char* mProcInfoName;
};

class BgmSuffixProcInfo : public BgmProcInfo {
public:

    static BgmSuffixProcInfo* createInfo(const ByamlIter& rIter, const char* pProcInfoName);

    const char* mSuffixName;
    bool mIsStartCurPosition;
};
static_assert(sizeof(BgmSuffixProcInfo) == 0x20);

class BgmVolumeProcInfo : public BgmProcInfo {
public:

    static BgmVolumeProcInfo* createInfo(const ByamlIter& rIter, const char* pProcInfoName);

    f32 mTargetVolume;
    f32 mVolumeDiff;
};
static_assert(sizeof(BgmVolumeProcInfo) == 0x18);

class BgmTrackChangeInfo {
public:
    static s32 compareInfo(const BgmTrackChangeInfo* pA, const BgmTrackChangeInfo* pB);

    s32 mTrackNo;
    f32 mVolume;
    s32 mFadeFrameNum;
};
static_assert(sizeof(BgmTrackChangeInfo) == 0xc);

class BgmTrackProcInfo : public BgmProcInfo {
public:

    static BgmTrackProcInfo* createInfo(const ByamlIter& rIter, const char* pProcInfoName);

    AudioInfoList<BgmTrackChangeInfo>* mChangeTrackInfoList;
};
static_assert(sizeof(BgmTrackProcInfo) == 0x18);

class BgmRegionProcInfo : public BgmProcInfo {
public:

    static BgmRegionProcInfo* createInfo(const ByamlIter& rIter, const char* pProcInfoName);

    s32 mHeadNo;
    s32 mLoopStartNo;
    s32 mLoopEndNo;
    bool mIsLoop;
    bool mIsPlayHeadOneTime;
    const char* mNextSituationName;
};
static_assert(sizeof(BgmRegionProcInfo) == 0x28);

class BgmPitchProcInfo : public BgmProcInfo {
public:

    static BgmPitchProcInfo* createInfo(const ByamlIter& rIter, const char* pProcInfoName);

    f32 mTargetPitch;
    f32 mPitchDiff;
};
static_assert(sizeof(BgmPitchProcInfo) == 0x18);

class BgmPitchModulationProcInfo : public BgmProcInfo {
public:

    static BgmPitchModulationProcInfo* createInfo(const ByamlIter& rIter, const char* pProcInfoName);

    bool mIsEnable;
    f32 mModDepth;
    f32 mModDepthDiff;
    f32 mModFreq;
};
static_assert(sizeof(BgmPitchModulationProcInfo) == 0x20);

class BgmLpfProcInfo : public BgmProcInfo {
public:

    static BgmLpfProcInfo* createInfo(const ByamlIter& rIter, const char* pProcInfoName);

    f32 mCutOffFreq;
    f32 mCutOffFreqDiff;
};
static_assert(sizeof(BgmLpfProcInfo) == 0x18);

class BgmMoveLoopStartProcInfo : public BgmProcInfo {
public:

    static BgmMoveLoopStartProcInfo* createInfo(const ByamlIter& rIter, const char* pProcInfoName);
};
static_assert(sizeof(BgmMoveLoopStartProcInfo) == 0x10);

class BgmSubSituationInfo {
public:
    static BgmSubSituationInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const BgmSubSituationInfo* pA, const BgmSubSituationInfo* pB);

    const char* mName;
    AudioInfoList<BgmProcInfo>* mProcInfoList;
};
static_assert(sizeof(BgmSubSituationInfo) == 0x10);

class BgmSituationInfo {
public:
    static BgmSituationInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const BgmSituationInfo* pA, const BgmSituationInfo* pB);

    const char* mName;
    AudioInfoList<BgmSubSituationInfo>* mSubSituationInfoList;
};
static_assert(sizeof(BgmSituationInfo) == 0x10);

class BgmDataBase {
public:
    BgmDataBase();

    AudioInfoList<BgmCombinedLineInfo>* mCombinedLineInfoList = nullptr;
    AudioInfoList<BgmPlayInfo>* mPlayInfoList = nullptr;
    AudioInfoList<BgmResourceInfo>* mResourceInfoList = nullptr;
    AudioInfoList<BgmStageInfo>* mStageInfoList = nullptr;
    AudioInfoList<BgmSituationInfo>* mSituationInfoList = nullptr;
    sead::PtrArray<BgmUserInfo>* mUserInfoList;
};
static_assert(sizeof(BgmDataBase) == 0x30);
}  // namespace al

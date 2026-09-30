#pragma once

#include <basis/seadTypes.h>

#include "Project/Audio/AudioInfoList.hpp"

namespace al {
class ByamlIter;

class SeEffectProcInfo {
public:
    static SeEffectProcInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const SeEffectProcInfo* pA, const SeEffectProcInfo* pB);

    virtual void dummy() {}

    const char* mName;
};

class SeDelayEffectProcInfo : public SeEffectProcInfo {
public:
    SeDelayEffectProcInfo();

    static SeDelayEffectProcInfo* createInfo(const ByamlIter& rIter, const char* pName);

    f32 mDelayTime = 0.0f;
    f32 mFeedbackGain = 0.0f;
    f32 mOutGain = 0.0f;
    f32 mLpfCutoffFreq = 0.0f;
    s32 mMaxChannels = 0;
    s32 mSampleRate = 0;
    bool mIsUseTaskThread = false;
    s32 mNumOfWaveBuffer = 0;
    s32 mNumOfPreloadWaveBuffer = 0;
};

static_assert(sizeof(SeDelayEffectProcInfo) == 0x38);

class SeReverbStdEffectProcInfo : public SeEffectProcInfo {
public:
    SeReverbStdEffectProcInfo();

    static SeReverbStdEffectProcInfo* createInfo(const ByamlIter& rIter, const char* pName);

    f32 mPreDelayTime = 0.0f;
    f32 mFusedTime = 0.0f;
    f32 mColoration = 0.0f;
    f32 mDamping = 0.0f;
    f32 mOutGain = 0.0f;
    s32 mEarlyMode = 5;
    s32 mFusedMode = 0;
    f32 mEarlyGain = 0.0f;
    f32 mFusedGain = 0.0f;
    s32 mMaxChannels = 0;
    s32 mSampleRate = 0;
    bool mIsUseTaskThread = false;
    s32 mNumOfWaveBuffer = 0;
    s32 mNumOfPreloadWaveBuffer = 0;
};

static_assert(sizeof(SeReverbStdEffectProcInfo) == 0x48);

class SeReverbHiEffectProcInfo : public SeEffectProcInfo {
public:
    SeReverbHiEffectProcInfo();

    static SeReverbHiEffectProcInfo* createInfo(const ByamlIter& rIter, const char* pName);

    f32 mPreDelayTime = 0.0f;
    f32 mFusedTime = 0.0f;
    f32 mColoration = 0.0f;
    f32 mDamping = 0.0f;
    f32 mCrosstalk = 0.0f;
    f32 mOutGain = 0.0f;
    s32 mEarlyMode = 5;
    s32 mFusedMode = 0;
    f32 mEarlyGain = 0.0f;
    f32 mFusedGain = 0.0f;
    s32 mMaxChannels = 0;
    s32 mSampleRate = 0;
    bool mIsUseTaskThread = false;
    s32 mNumOfWaveBuffer = 0;
    s32 mNumOfPreloadWaveBuffer = 0;
};

static_assert(sizeof(SeReverbHiEffectProcInfo) == 0x50);

class SeReverbI3Dl2EffectProcInfo : public SeEffectProcInfo {
public:
    SeReverbI3Dl2EffectProcInfo();

    static SeReverbI3Dl2EffectProcInfo* createInfo(const ByamlIter& rIter, const char* pName);

    s32 mRoom = 0;
    s32 mRoomHf = 0;
    f32 mDecayTime = 0.0f;
    f32 mDecayHfRatio = 0.0f;
    s32 mReflections = 0;
    f32 mReflectionsDelay = 0.0f;
    s32 mReverb = 0;
    f32 mReverbDelay = 0.0f;
    f32 mDiffusion = 0.0f;
    f32 mDensity = 0.0f;
    f32 mHfReference = 0.0f;
    s32 mEarlyMode = 5;
    s32 mFusedMode = 0;
    s32 mMaxChannels = 0;
    s32 mSampleRate = 0;
    bool mIsUseTaskThread = false;
    s32 mNumOfWaveBuffer = 0;
    s32 mNumOfPreloadWaveBuffer = 0;
};

static_assert(sizeof(SeReverbI3Dl2EffectProcInfo) == 0x58);

class SeChorusEffectProcInfo : public SeEffectProcInfo {
public:
    SeChorusEffectProcInfo();

    static SeChorusEffectProcInfo* createInfo(const ByamlIter& rIter, const char* pName);

    f32 mDelayTime = 0.0f;
    f32 mDepth = 0.0f;
    f32 mRate = 0.0f;
    f32 mFeedback = 0.0f;
    f32 mOutGain = 0.0f;
};

static_assert(sizeof(SeChorusEffectProcInfo) == 0x28);

class SeLpfEffectProcInfo : public SeEffectProcInfo {
public:
    SeLpfEffectProcInfo();

    static SeLpfEffectProcInfo* createInfo(const ByamlIter& rIter, const char* pName);

    f32 mLpfFreq = 0.0f;
};

static_assert(sizeof(SeLpfEffectProcInfo) == 0x18);

class SeUseEffectInfo {
public:
    SeUseEffectInfo();

    static SeUseEffectInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const SeUseEffectInfo* pA, const SeUseEffectInfo* pB);

    const char* mName = nullptr;
};

class SeEffectBusUserInfo {
public:
    static const char* DEFAULT_CATEGORY_NAME;

    SeEffectBusUserInfo();

    static SeEffectBusUserInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const SeEffectBusUserInfo* pA, const SeEffectBusUserInfo* pB);

    const char* mName = nullptr;
    const char* mCategoryName = DEFAULT_CATEGORY_NAME;
    f32 mMainOutputSend = 0.0f;
    f32 mSubOutputSend = 0.0f;
};

static_assert(sizeof(SeEffectBusUserInfo) == 0x18);

class SeEffectBusInfo {
public:
    SeEffectBusInfo();

    static SeEffectBusInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const SeEffectBusInfo* pA, const SeEffectBusInfo* pB);

    const char* mName = nullptr;
    AudioInfoList<SeEffectBusUserInfo>* mEffectBusUserInfoList = nullptr;
};

class SeEffectBusSettingInfo {
public:
    SeEffectBusSettingInfo();

    static SeEffectBusSettingInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const SeEffectBusSettingInfo* pA, const SeEffectBusSettingInfo* pB);

    const char* mName = nullptr;
    AudioInfoList<SeEffectBusInfo>* mEffectBusInfoList = nullptr;
};

class AudioEachBusEffectInfo {
public:
    AudioEachBusEffectInfo();

    static AudioEachBusEffectInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const AudioEachBusEffectInfo* pA, const AudioEachBusEffectInfo* pB);

    const char* mName = nullptr;
    AudioInfoList<SeEffectProcInfo>* mEffectProcInfoList = nullptr;
};

class SeEffectInfo {
public:
    SeEffectInfo();

    static SeEffectInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const SeEffectInfo* pA, const SeEffectInfo* pB);

    const char* mName = nullptr;
    AudioInfoList<AudioEachBusEffectInfo>* mEachBusEffectInfoList = nullptr;
};

class SeStageEffectInfo {
public:
    SeStageEffectInfo();

    static SeStageEffectInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const SeStageEffectInfo* pA, const SeStageEffectInfo* pB);

    const char* mName = nullptr;
    const char* mEffectBusSettingName = nullptr;
    AudioInfoList<SeUseEffectInfo>* mUseEffectInfoList = nullptr;
};

static_assert(sizeof(SeStageEffectInfo) == 0x18);
}  // namespace al

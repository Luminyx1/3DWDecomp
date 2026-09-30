#include "Project/Audio/AudioBusInfo.hpp"

#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Base/StringUtil.hpp"

namespace {
/**
 * Converts an early reflection mode name to its id.
 * @param pName Early reflection mode name.
 * @return Early reflection mode id.
 */
s32 convertEarlyMode(const char* pName) {
    if (al::isEqualString(pName, "EARLY_REFLECTION_5MS")) {
        return 0;
    }
    if (al::isEqualString(pName, "EARLY_REFLECTION_10MS")) {
        return 1;
    }
    if (al::isEqualString(pName, "EARLY_REFLECTION_15MS")) {
        return 2;
    }
    if (al::isEqualString(pName, "EARLY_REFLECTION_20MS")) {
        return 3;
    }
    if (al::isEqualString(pName, "EARLY_REFLECTION_25MS")) {
        return 4;
    }
    if (al::isEqualString(pName, "EARLY_REFLECTION_30MS")) {
        return 5;
    }
    if (al::isEqualString(pName, "EARLY_REFLECTION_35MS")) {
        return 6;
    }
    if (al::isEqualString(pName, "EARLY_REFLECTION_40MS")) {
        return 7;
    }
    return 5;
}

/**
 * Converts a fused mode name to its id.
 * @param pName Fused mode name.
 * @return Fused mode id.
 */
inline s32 convertFusedMode(const char* pName) {
    if (al::isEqualString(pName, "FUSED_OLD_AXFX")) {
        return 0;
    }
    if (al::isEqualString(pName, "FUSED_METAL_TANK")) {
        return 1;
    }
    if (al::isEqualString(pName, "FUSED_SMALL_ROOM")) {
        return 2;
    }
    if (al::isEqualString(pName, "FUSED_LARGE_ROOM")) {
        return 3;
    }
    if (al::isEqualString(pName, "FUSED_HALL")) {
        return 4;
    }
    return al::isEqualString(pName, "FUSED_CAVERNOUS") ? 5 : 0;
}

}  // namespace

namespace al {
/**
 * Constructs empty delay effect information.
 */
SeDelayEffectProcInfo::SeDelayEffectProcInfo() = default;

/**
 * Constructs empty standard reverb effect information.
 */
SeReverbStdEffectProcInfo::SeReverbStdEffectProcInfo() = default;

/**
 * Constructs empty high quality reverb effect information.
 */
SeReverbHiEffectProcInfo::SeReverbHiEffectProcInfo() = default;

/**
 * Constructs empty I3DL2 reverb effect information.
 */
SeReverbI3Dl2EffectProcInfo::SeReverbI3Dl2EffectProcInfo() = default;

/**
 * Constructs empty chorus effect information.
 */
SeChorusEffectProcInfo::SeChorusEffectProcInfo() = default;

/**
 * Constructs empty low pass filter effect information.
 */
SeLpfEffectProcInfo::SeLpfEffectProcInfo() = default;

/**
 * Constructs empty used effect information.
 */
SeUseEffectInfo::SeUseEffectInfo() = default;

/**
 * Constructs empty effect bus user information.
 */
SeEffectBusUserInfo::SeEffectBusUserInfo() = default;

/**
 * Constructs empty effect bus information.
 */
SeEffectBusInfo::SeEffectBusInfo() = default;

/**
 * Constructs empty effect bus setting information.
 */
SeEffectBusSettingInfo::SeEffectBusSettingInfo() = default;

/**
 * Constructs empty bus effect information.
 */
AudioEachBusEffectInfo::AudioEachBusEffectInfo() = default;

/**
 * Constructs empty effect information.
 */
SeEffectInfo::SeEffectInfo() = default;

/**
 * Constructs empty stage effect information.
 */
SeStageEffectInfo::SeStageEffectInfo() = default;

/**
 * Creates effect bus user information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information, or nullptr if it has no name.
 */
SeEffectBusUserInfo* SeEffectBusUserInfo::createInfo(const ByamlIter& rIter) {
    SeEffectBusUserInfo* info = new SeEffectBusUserInfo;
    if (!rIter.tryGetStringByKey(&info->mName, "Name")) {
        return nullptr;
    }
    if (!rIter.tryGetStringByKey(&info->mCategoryName, "CategoryName")) {
        info->mCategoryName = DEFAULT_CATEGORY_NAME;
    }
    if (!rIter.tryGetFloatByKey(&info->mMainOutputSend, "MainOutputSend")) {
        info->mMainOutputSend = 1.0f;
    }
    if (!rIter.tryGetFloatByKey(&info->mSubOutputSend, "SubOutputSend")) {
        info->mSubOutputSend = 1.0f;
    }
    return info;
}

/**
 * Creates effect bus information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information, or nullptr if it has no name.
 */
SeEffectBusInfo* SeEffectBusInfo::createInfo(const ByamlIter& rIter) {
    SeEffectBusInfo* info = new SeEffectBusInfo;
    if (!rIter.tryGetStringByKey(&info->mName, "Name")) {
        return nullptr;
    }
    ByamlIter userIter;
    if (rIter.tryGetIterByKey(&userIter, "EffectBusUserInfoList")) {
        info->mEffectBusUserInfoList = createInfoList<SeEffectBusUserInfo>(userIter);
    } else {
        info->mEffectBusUserInfoList = nullptr;
    }
    return info;
}

/**
 * Creates effect bus setting information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information, or nullptr if it has no name.
 */
SeEffectBusSettingInfo* SeEffectBusSettingInfo::createInfo(const ByamlIter& rIter) {
    SeEffectBusSettingInfo* info = new SeEffectBusSettingInfo;
    if (!rIter.tryGetStringByKey(&info->mName, "Name")) {
        return nullptr;
    }
    ByamlIter busIter;
    if (rIter.tryGetIterByKey(&busIter, "EffectBusInfoList")) {
        info->mEffectBusInfoList = createInfoList<SeEffectBusInfo>(busIter);
    } else {
        info->mEffectBusInfoList = nullptr;
    }
    return info;
}

/**
 * Creates effect process information of the type given in the BYAML data.
 * @param rIter BYAML data.
 * @return Created information, or nullptr for unknown types.
 */
SeEffectProcInfo* SeEffectProcInfo::createInfo(const ByamlIter& rIter) {
    const char* name = nullptr;
    if (!rIter.tryGetStringByKey(&name, "Name")) {
        return nullptr;
    }
    if (isEqualString(name, "Delay")) {
        return SeDelayEffectProcInfo::createInfo(rIter, name);
    }
    if (isEqualString(name, "ReverbStd")) {
        return SeReverbStdEffectProcInfo::createInfo(rIter, name);
    }
    if (isEqualString(name, "ReverbHi")) {
        return SeReverbHiEffectProcInfo::createInfo(rIter, name);
    }
    if (isEqualString(name, "ReverbI3Dl2")) {
        return SeReverbI3Dl2EffectProcInfo::createInfo(rIter, name);
    }
    if (isEqualString(name, "Chorus")) {
        return SeChorusEffectProcInfo::createInfo(rIter, name);
    }
    if (isEqualString(name, "Lpf")) {
        return SeLpfEffectProcInfo::createInfo(rIter, name);
    }
    return nullptr;
}

/**
 * Creates delay effect information from BYAML data.
 * @param rIter BYAML data.
 * @param pName Effect name.
 * @return Created information.
 */
SeDelayEffectProcInfo* SeDelayEffectProcInfo::createInfo(const ByamlIter& rIter, const char* pName) {
    SeDelayEffectProcInfo* info = new SeDelayEffectProcInfo;
    info->mName = pName;
    if (!rIter.tryGetFloatByKey(&info->mDelayTime, "DelayTime")) {
        info->mDelayTime = 160.0f;
    }
    if (!rIter.tryGetFloatByKey(&info->mFeedbackGain, "FeedbackGain")) {
        info->mFeedbackGain = 0.4f;
    }
    if (!rIter.tryGetFloatByKey(&info->mOutGain, "OutGain")) {
        info->mOutGain = 1.0f;
    }
    if (!rIter.tryGetFloatByKey(&info->mLpfCutoffFreq, "LpfCutoffFreq")) {
        info->mLpfCutoffFreq = 1.0f;
    }
    if (!rIter.tryGetIntByKey(&info->mMaxChannels, "MaxChannels")) {
        info->mMaxChannels = 2;
    }
    const char* sampleRateName = nullptr;
    info->mSampleRate =
        rIter.tryGetStringByKey(&sampleRateName, "SampleRate") ? isEqualString(sampleRateName, "SampleRate48k") : 0;
    if (!rIter.tryGetBoolByKey(&info->mIsUseTaskThread, "IsUseTaskThread")) {
        info->mIsUseTaskThread = false;
    }
    if (!rIter.tryGetIntByKey(&info->mNumOfWaveBuffer, "NumOfWaveBuffer")) {
        info->mNumOfWaveBuffer = 2;
    }
    if (!rIter.tryGetIntByKey(&info->mNumOfPreloadWaveBuffer, "NumOfPreloadWaveBuffer")) {
        info->mNumOfPreloadWaveBuffer = 1;
    }
    return info;
}

/**
 * Creates standard reverb effect information from BYAML data.
 * @param rIter BYAML data.
 * @param pName Effect name.
 * @return Created information.
 */
SeReverbStdEffectProcInfo* SeReverbStdEffectProcInfo::createInfo(const ByamlIter& rIter, const char* pName) {
    SeReverbStdEffectProcInfo* info = new SeReverbStdEffectProcInfo;
    info->mName = pName;
    if (!rIter.tryGetFloatByKey(&info->mPreDelayTime, "PreDelayTime")) {
        info->mPreDelayTime = 0.2f;
    }
    if (!rIter.tryGetFloatByKey(&info->mFusedTime, "FusedTime")) {
        info->mFusedTime = 3.0f;
    }
    if (!rIter.tryGetFloatByKey(&info->mColoration, "Coloration")) {
        info->mColoration = 0.6f;
    }
    if (!rIter.tryGetFloatByKey(&info->mDamping, "Damping")) {
        info->mDamping = 0.4f;
    }
    if (!rIter.tryGetFloatByKey(&info->mOutGain, "OutGain")) {
        info->mOutGain = 1.0f;
    }
    const char* earlyModeName = nullptr;
    info->mEarlyMode = rIter.tryGetStringByKey(&earlyModeName, "EarlyMode") ? convertEarlyMode(earlyModeName) : 5;
    const char* fusedModeName = nullptr;
    info->mFusedMode = rIter.tryGetStringByKey(&fusedModeName, "FusedMode") ? convertFusedMode(fusedModeName) : 0;
    if (!rIter.tryGetFloatByKey(&info->mEarlyGain, "EarlyGain")) {
        info->mEarlyGain = 0.0f;
    }
    if (!rIter.tryGetFloatByKey(&info->mFusedGain, "FusedGain")) {
        info->mFusedGain = 1.0f;
    }
    if (!rIter.tryGetIntByKey(&info->mMaxChannels, "MaxChannels")) {
        info->mMaxChannels = 2;
    }
    const char* sampleRateName = nullptr;
    info->mSampleRate =
        rIter.tryGetStringByKey(&sampleRateName, "SampleRate") ? isEqualString(sampleRateName, "SampleRate48k") : 0;
    if (!rIter.tryGetBoolByKey(&info->mIsUseTaskThread, "IsUseTaskThread")) {
        info->mIsUseTaskThread = false;
    }
    if (!rIter.tryGetIntByKey(&info->mNumOfWaveBuffer, "NumOfWaveBuffer")) {
        info->mNumOfWaveBuffer = 2;
    }
    if (!rIter.tryGetIntByKey(&info->mNumOfPreloadWaveBuffer, "NumOfPreloadWaveBuffer")) {
        info->mNumOfPreloadWaveBuffer = 1;
    }
    return info;
}

/**
 * Creates high quality reverb effect information from BYAML data.
 * @param rIter BYAML data.
 * @param pName Effect name.
 * @return Created information.
 */
SeReverbHiEffectProcInfo* SeReverbHiEffectProcInfo::createInfo(const ByamlIter& rIter, const char* pName) {
    SeReverbHiEffectProcInfo* info = new SeReverbHiEffectProcInfo;
    info->mName = pName;
    if (!rIter.tryGetFloatByKey(&info->mPreDelayTime, "PreDelayTime")) {
        info->mPreDelayTime = 0.02f;
    }
    if (!rIter.tryGetFloatByKey(&info->mFusedTime, "FusedTime")) {
        info->mFusedTime = 3.0f;
    }
    if (!rIter.tryGetFloatByKey(&info->mColoration, "Coloration")) {
        info->mColoration = 0.6f;
    }
    if (!rIter.tryGetFloatByKey(&info->mDamping, "Damping")) {
        info->mDamping = 0.4f;
    }
    if (!rIter.tryGetFloatByKey(&info->mCrosstalk, "Crosstalk")) {
        info->mCrosstalk = 0.1f;
    }
    if (!rIter.tryGetFloatByKey(&info->mOutGain, "OutGain")) {
        info->mOutGain = 1.0f;
    }
    const char* earlyModeName = nullptr;
    info->mEarlyMode = rIter.tryGetStringByKey(&earlyModeName, "EarlyMode") ? convertEarlyMode(earlyModeName) : 5;
    const char* fusedModeName = nullptr;
    info->mFusedMode = rIter.tryGetStringByKey(&fusedModeName, "FusedMode") ? convertFusedMode(fusedModeName) : 0;
    if (!rIter.tryGetFloatByKey(&info->mEarlyGain, "EarlyGain")) {
        info->mEarlyGain = 0.0f;
    }
    if (!rIter.tryGetFloatByKey(&info->mFusedGain, "FusedGain")) {
        info->mFusedGain = 1.0f;
    }
    if (!rIter.tryGetIntByKey(&info->mMaxChannels, "MaxChannels")) {
        info->mMaxChannels = 2;
    }
    const char* sampleRateName = nullptr;
    info->mSampleRate =
        rIter.tryGetStringByKey(&sampleRateName, "SampleRate") ? isEqualString(sampleRateName, "SampleRate48k") : 0;
    if (!rIter.tryGetBoolByKey(&info->mIsUseTaskThread, "IsUseTaskThread")) {
        info->mIsUseTaskThread = false;
    }
    if (!rIter.tryGetIntByKey(&info->mNumOfWaveBuffer, "NumOfWaveBuffer")) {
        info->mNumOfWaveBuffer = 2;
    }
    if (!rIter.tryGetIntByKey(&info->mNumOfPreloadWaveBuffer, "NumOfPreloadWaveBuffer")) {
        info->mNumOfPreloadWaveBuffer = 1;
    }
    return info;
}

/**
 * Creates I3DL2 reverb effect information from BYAML data.
 * @param rIter BYAML data.
 * @param pName Effect name.
 * @return Created information.
 */
SeReverbI3Dl2EffectProcInfo* SeReverbI3Dl2EffectProcInfo::createInfo(const ByamlIter& rIter, const char* pName) {
    SeReverbI3Dl2EffectProcInfo* info = new SeReverbI3Dl2EffectProcInfo;
    info->mName = pName;
    if (!rIter.tryGetIntByKey(&info->mRoom, "Room")) {
        info->mRoom = -1000;
    }
    if (!rIter.tryGetIntByKey(&info->mRoomHf, "RoomHf")) {
        info->mRoomHf = 0;
    }
    if (!rIter.tryGetFloatByKey(&info->mDecayTime, "DecayTime")) {
        info->mDecayTime = 1.0f;
    }
    if (!rIter.tryGetFloatByKey(&info->mDecayHfRatio, "DecayHfRatio")) {
        info->mDecayHfRatio = 0.5f;
    }
    if (!rIter.tryGetIntByKey(&info->mReflections, "Reflections")) {
        info->mReflections = -1000;
    }
    if (!rIter.tryGetFloatByKey(&info->mReflectionsDelay, "ReflectionsDelay")) {
        info->mReflectionsDelay = 0.02f;
    }
    if (!rIter.tryGetIntByKey(&info->mReverb, "Reverb")) {
        info->mReverb = -1000;
    }
    if (!rIter.tryGetFloatByKey(&info->mReverbDelay, "ReverbDelay")) {
        info->mReverbDelay = 0.04f;
    }
    if (!rIter.tryGetFloatByKey(&info->mDiffusion, "Diffusion")) {
        info->mDiffusion = 100.0f;
    }
    if (!rIter.tryGetFloatByKey(&info->mDensity, "Density")) {
        info->mDensity = 100.0f;
    }
    if (!rIter.tryGetFloatByKey(&info->mHfReference, "HfReference")) {
        info->mHfReference = 5000.0f;
    }
    const char* earlyModeName = nullptr;
    info->mEarlyMode = rIter.tryGetStringByKey(&earlyModeName, "EarlyMode") ? convertEarlyMode(earlyModeName) : 5;
    const char* fusedModeName = nullptr;
    info->mFusedMode = rIter.tryGetStringByKey(&fusedModeName, "FusedMode") ? convertFusedMode(fusedModeName) : 0;
    if (!rIter.tryGetIntByKey(&info->mMaxChannels, "MaxChannels")) {
        info->mMaxChannels = 2;
    }
    const char* sampleRateName = nullptr;
    info->mSampleRate =
        rIter.tryGetStringByKey(&sampleRateName, "SampleRate") ? isEqualString(sampleRateName, "SampleRate48k") : 0;
    if (!rIter.tryGetBoolByKey(&info->mIsUseTaskThread, "IsUseTaskThread")) {
        info->mIsUseTaskThread = false;
    }
    if (!rIter.tryGetIntByKey(&info->mNumOfWaveBuffer, "NumOfWaveBuffer")) {
        info->mNumOfWaveBuffer = 2;
    }
    if (!rIter.tryGetIntByKey(&info->mNumOfPreloadWaveBuffer, "NumOfPreloadWaveBuffer")) {
        info->mNumOfPreloadWaveBuffer = 1;
    }
    return info;
}

/**
 * Creates chorus effect information from BYAML data.
 * @param rIter BYAML data.
 * @param pName Effect name.
 * @return Created information.
 */
SeChorusEffectProcInfo* SeChorusEffectProcInfo::createInfo(const ByamlIter& rIter, const char* pName) {
    SeChorusEffectProcInfo* info = new SeChorusEffectProcInfo;
    info->mName = pName;
    if (!rIter.tryGetFloatByKey(&info->mDelayTime, "DelayTime")) {
        info->mDelayTime = 10.0f;
    }
    if (!rIter.tryGetFloatByKey(&info->mDepth, "Depth")) {
        info->mDepth = 0.5f;
    }
    if (!rIter.tryGetFloatByKey(&info->mRate, "Rate")) {
        info->mRate = 1.0f;
    }
    if (!rIter.tryGetFloatByKey(&info->mFeedback, "Feedback")) {
        info->mFeedback = 0.0f;
    }
    if (!rIter.tryGetFloatByKey(&info->mOutGain, "OutGain")) {
        info->mOutGain = 1.0f;
    }
    return info;
}

/**
 * Creates low pass filter effect information from BYAML data.
 * @param rIter BYAML data.
 * @param pName Effect name.
 * @return Created information.
 */
SeLpfEffectProcInfo* SeLpfEffectProcInfo::createInfo(const ByamlIter& rIter, const char* pName) {
    SeLpfEffectProcInfo* info = new SeLpfEffectProcInfo;
    info->mName = pName;
    rIter.tryGetFloatByKey(&info->mLpfFreq, "LpfFreq");
    return info;
}

AudioEachBusEffectInfo* AudioEachBusEffectInfo::createInfo(const ByamlIter& rIter) {
    AudioEachBusEffectInfo* info = new AudioEachBusEffectInfo;
    rIter.tryGetStringByKey(&info->mName, "Name");
    ByamlIter procIter;
    if (!rIter.tryGetIterByKey(&procIter, "EffectProcInfoList")) {
        info->mEffectProcInfoList = nullptr;
        return nullptr;
    }
    info->mEffectProcInfoList = createInfoList<SeEffectProcInfo>(procIter);
    return info;
}

SeEffectInfo* SeEffectInfo::createInfo(const ByamlIter& rIter) {
    SeEffectInfo* info = new SeEffectInfo;
    rIter.tryGetStringByKey(&info->mName, "Name");
    ByamlIter busIter;
    if (!rIter.tryGetIterByKey(&busIter, "EachBusEffectInfoList")) {
        info->mEachBusEffectInfoList = nullptr;
        return nullptr;
    }
    info->mEachBusEffectInfoList = createInfoList<AudioEachBusEffectInfo>(busIter);
    return info;
}

/**
 * Creates used effect information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information, or nullptr if it has no name.
 */
SeUseEffectInfo* SeUseEffectInfo::createInfo(const ByamlIter& rIter) {
    SeUseEffectInfo* info = new SeUseEffectInfo;
    return rIter.tryGetStringByKey(&info->mName, "Name") ? info : nullptr;
}

/**
 * Creates stage effect information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information, or nullptr if it has no used effect list.
 */
SeStageEffectInfo* SeStageEffectInfo::createInfo(const ByamlIter& rIter) {
    SeStageEffectInfo* info = new SeStageEffectInfo;
    rIter.tryGetStringByKey(&info->mName, "Name");
    ByamlIter useIter;
    if (!rIter.tryGetStringByKey(&info->mEffectBusSettingName, "EffectBusSettingName")) {
        info->mEffectBusSettingName = "Default";
    }
    if (!rIter.tryGetIterByKey(&useIter, "UseEffectInfoList")) {
        info->mUseEffectInfoList = nullptr;
        return nullptr;
    }
    info->mUseEffectInfoList = createInfoList<SeUseEffectInfo>(useIter);
    return info;
}

/**
 * Compares two effect process information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 SeEffectProcInfo::compareInfo(const SeEffectProcInfo* pA, const SeEffectProcInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

/**
 * Compares two used effect information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 SeUseEffectInfo::compareInfo(const SeUseEffectInfo* pA, const SeUseEffectInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

/**
 * Compares two effect bus user information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 SeEffectBusUserInfo::compareInfo(const SeEffectBusUserInfo* pA, const SeEffectBusUserInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

/**
 * Compares two effect bus information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 SeEffectBusInfo::compareInfo(const SeEffectBusInfo* pA, const SeEffectBusInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

/**
 * Compares two effect bus setting information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 SeEffectBusSettingInfo::compareInfo(const SeEffectBusSettingInfo* pA, const SeEffectBusSettingInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

/**
 * Compares two bus effect information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 AudioEachBusEffectInfo::compareInfo(const AudioEachBusEffectInfo* pA, const AudioEachBusEffectInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

/**
 * Compares two effect information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 SeEffectInfo::compareInfo(const SeEffectInfo* pA, const SeEffectInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

/**
 * Compares two stage effect information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 SeStageEffectInfo::compareInfo(const SeStageEffectInfo* pA, const SeStageEffectInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}
}  // namespace al

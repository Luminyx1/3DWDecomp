#include "Library/Bgm/BgmDataBase.hpp"

#include "Library/Yaml/ByamlIter.hpp"

namespace al {
/**
 * Creates BGM resource suffix information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information.
 */
BgmResourceSuffixInfo* BgmResourceSuffixInfo::createInfo(const ByamlIter& rIter) {
    BgmResourceSuffixInfo* info = new BgmResourceSuffixInfo();
    rIter.tryGetStringByKey(&info->mSuffixName, "SuffixName");
    if (!rIter.tryGetIntByKey(&info->mStartSample, "StartSample")) {
        info->mStartSample = 0;
    }
    if (!rIter.tryGetFloatByKey(&info->mBpm, "Bpm")) {
        info->mBpm = 1.0f;
    }
    if (!rIter.tryGetIntByKey(&info->mSampleRate, "SampleRate")) {
        info->mSampleRate = 32000;
    }
    return info;
}
/**
 * Creates BGM enable situation information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information.
 */
BgmEnableSituationInfo* BgmEnableSituationInfo::createInfo(const ByamlIter& rIter) {
    BgmEnableSituationInfo* info = new BgmEnableSituationInfo();
    rIter.tryGetStringByKey(&info->mName, "Name");
    if (!rIter.tryGetStringByKey(&info->mSubSituationName, "SubSituationName")) {
        info->mSubSituationName = "Default";
    }
    return info;
}
/**
 * Creates BGM start trigger situation information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information.
 */
BgmStartTriggerSituationInfo* BgmStartTriggerSituationInfo::createInfo(const ByamlIter& rIter) {
    BgmStartTriggerSituationInfo* info = new BgmStartTriggerSituationInfo();
    rIter.tryGetStringByKey(&info->mName, "Name");
    return info;
}
/**
 * Creates BGM resource information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information.
 */
BgmResourceInfo* BgmResourceInfo::createInfo(const ByamlIter& rIter) {
    BgmResourceInfo* info = new BgmResourceInfo();
    rIter.tryGetStringByKey(&info->mName, "Name");
    if (!rIter.tryGetIntByKey(&info->mStartSample, "StartSample")) {
        info->mStartSample = 0;
    }
    if (!rIter.tryGetFloatByKey(&info->mBpm, "Bpm")) {
        info->mBpm = 1.0f;
    }
    if (!rIter.tryGetIntByKey(&info->mSampleRate, "SampleRate")) {
        info->mSampleRate = 32000;
    }
    ByamlIter suffixIter;
    info->mResourceSuffixInfoList = nullptr;
    if (rIter.tryGetIterByKey(&suffixIter, "ResourceSuffixInfoList")) {
        info->mResourceSuffixInfoList = createInfoList<BgmResourceSuffixInfo>(suffixIter);
    }
    ByamlIter enableIter;
    info->mEnableSituationInfoList = nullptr;
    if (rIter.tryGetIterByKey(&enableIter, "EnableSituationInfoList")) {
        info->mEnableSituationInfoList = createInfoList<BgmEnableSituationInfo>(enableIter);
    }
    ByamlIter startTriggerIter;
    info->mStartTriggerSituationInfoList = nullptr;
    if (rIter.tryGetIterByKey(&startTriggerIter, "StartTriggerSituationInfoList")) {
        info->mStartTriggerSituationInfoList = createInfoList<BgmStartTriggerSituationInfo>(startTriggerIter);
    }
    if (!rIter.tryGetBoolByKey(&info->mIsEnableRegionJump, "IsEnableRegionJump")) {
        info->mIsEnableRegionJump = false;
    }
    if (!rIter.tryGetBoolByKey(&info->mIsDisableAudioEffect, "IsDisableAudioEffect")) {
        info->mIsDisableAudioEffect = false;
    }
    if (!rIter.tryGetBoolByKey(&info->mIsEnableNwRender, "IsEnableNwRender")) {
        info->mIsEnableNwRender = false;
    }
    return info;
}
/**
 * Compares two BGM resource suffix information by suffix name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 BgmResourceSuffixInfo::compareInfo(const BgmResourceSuffixInfo* pA, const BgmResourceSuffixInfo* pB) {
    return strcmp(pA->mSuffixName, pB->mSuffixName);
}
/**
 * Compares two BGM enable situation information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 BgmEnableSituationInfo::compareInfo(const BgmEnableSituationInfo* pA, const BgmEnableSituationInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}
/**
 * Compares two BGM start trigger situation information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 BgmStartTriggerSituationInfo::compareInfo(const BgmStartTriggerSituationInfo* pA,
                                              const BgmStartTriggerSituationInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}
/**
 * Compares two BGM resource information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 BgmResourceInfo::compareInfo(const BgmResourceInfo* pA, const BgmResourceInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}
}  // namespace al

#include "Project/Bgm/BgmResourceCategoryInfo.hpp"

#include <cstring>

#include "Library/Yaml/ByamlIter.hpp"

namespace al {
/**
 * @brief Creates the info of one resource suffix (an alternate version of a BGM resource).
 * @param rIter The yaml iterator of the suffix entry.
 * @return The new suffix info.
 */
BgmResourceSuffixInfo* BgmResourceSuffixInfo::createInfo(const ByamlIter& rIter) {
    BgmResourceSuffixInfo* info = new BgmResourceSuffixInfo();
    rIter.tryGetStringByKey(&info->mName, "SuffixName");
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
 * @brief Creates the info of a situation a BGM resource can play in.
 * @param rIter The yaml iterator of the situation entry.
 * @return The new situation info.
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
 * @brief Creates the info of a situation that starts a BGM resource.
 * @param rIter The yaml iterator of the situation entry.
 * @return The new situation info.
 */
BgmStartTriggerSituationInfo* BgmStartTriggerSituationInfo::createInfo(const ByamlIter& rIter) {
    BgmStartTriggerSituationInfo* info = new BgmStartTriggerSituationInfo();
    rIter.tryGetStringByKey(&info->mName, "Name");
    return info;
}

/**
 * @brief Creates the info of one BGM resource, including its suffix and situation lists.
 * @param rIter The yaml iterator of the resource entry.
 * @return The new resource info.
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
    info->mSuffixInfoList = nullptr;
    if (rIter.tryGetIterByKey(&suffixIter, "ResourceSuffixInfoList")) {
        info->mSuffixInfoList = createInfoList<BgmResourceSuffixInfo>(suffixIter);
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
 * @brief Orders two suffix infos by name.
 * @param pInfoA The first info.
 * @param pInfoB The second info.
 * @return Negative, zero or positive as the first name sorts before, equal to or after the second.
 */
s32 BgmResourceSuffixInfo::compareInfo(const BgmResourceSuffixInfo* pInfoA, const BgmResourceSuffixInfo* pInfoB) {
    return strcmp(pInfoA->mName, pInfoB->mName);
}

/**
 * @brief Orders two enable situation infos by name.
 * @param pInfoA The first info.
 * @param pInfoB The second info.
 * @return Negative, zero or positive as the first name sorts before, equal to or after the second.
 */
s32 BgmEnableSituationInfo::compareInfo(const BgmEnableSituationInfo* pInfoA, const BgmEnableSituationInfo* pInfoB) {
    return strcmp(pInfoA->mName, pInfoB->mName);
}

/**
 * @brief Orders two start trigger situation infos by name.
 * @param pInfoA The first info.
 * @param pInfoB The second info.
 * @return Negative, zero or positive as the first name sorts before, equal to or after the second.
 */
s32 BgmStartTriggerSituationInfo::compareInfo(const BgmStartTriggerSituationInfo* pInfoA,
                                              const BgmStartTriggerSituationInfo* pInfoB) {
    return strcmp(pInfoA->mName, pInfoB->mName);
}

/**
 * @brief Orders two resource infos by name.
 * @param pInfoA The first info.
 * @param pInfoB The second info.
 * @return Negative, zero or positive as the first name sorts before, equal to or after the second.
 */
s32 BgmResourceInfo::compareInfo(const BgmResourceInfo* pInfoA, const BgmResourceInfo* pInfoB) {
    return strcmp(pInfoA->mName, pInfoB->mName);
}
}  // namespace al

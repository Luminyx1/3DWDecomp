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
}  // namespace al

#include "Project/Bgm/BgmStageScenarioInfo.hpp"

#include <cstring>

#include "Library/Yaml/ByamlIter.hpp"

namespace al {
/**
 * @brief Creates the info of a BGM a stage plays.
 * @param rIter The yaml iterator of the stage play info entry.
 * @return The new stage play info.
 */
BgmStagePlayInfo* BgmStagePlayInfo::createInfo(const ByamlIter& rIter) {
    BgmStagePlayInfo* info = new BgmStagePlayInfo();
    rIter.tryGetStringByKey(&info->mName, "PlayInfoName");
    rIter.tryGetStringByKey(&info->mResourceName, "ResourceName");
    if (!rIter.tryGetStringByKey(&info->mRegionInfoListName, "RegionInfoListName")) {
        info->mRegionInfoListName = nullptr;
    }
    if (!rIter.tryGetIntByKey(&info->mStartDelayFrameNum, "StartDelayFrameNum")) {
        info->mStartDelayFrameNum = 0;
    }
    if (!rIter.tryGetIntByKey(&info->mFadeInFrameNum, "FadeInFrameNum")) {
        info->mFadeInFrameNum = 0;
    }
    return info;
}

/**
 * @brief Creates the info of the BGMs played in one stage.
 * @param rIter The yaml iterator of the stage entry.
 * @return The new stage info.
 */
BgmStageInfo* BgmStageInfo::createInfo(const ByamlIter& rIter) {
    BgmStageInfo* info = new BgmStageInfo();
    rIter.tryGetStringByKey(&info->mName, "Name");
    ByamlIter playInfoIter;
    rIter.tryGetIterByKey(&playInfoIter, "StagePlayInfo");
    info->mPlayInfoList = createInfoList<BgmStagePlayInfo>(playInfoIter);
    return info;
}

/**
 * @brief Orders two stage play infos by name.
 * @param pInfoA The first info.
 * @param pInfoB The second info.
 * @return Negative, zero or positive as the first name sorts before, equal to or after the second.
 */
s32 BgmStagePlayInfo::compareInfo(const BgmStagePlayInfo* pInfoA, const BgmStagePlayInfo* pInfoB) {
    return strcmp(pInfoA->mName, pInfoB->mName);
}

/**
 * @brief Orders two stage infos by name.
 * @param pInfoA The first info.
 * @param pInfoB The second info.
 * @return Negative, zero or positive as the first name sorts before, equal to or after the second.
 */
s32 BgmStageInfo::compareInfo(const BgmStageInfo* pInfoA, const BgmStageInfo* pInfoB) {
    return strcmp(pInfoA->mName, pInfoB->mName);
}
}  // namespace al

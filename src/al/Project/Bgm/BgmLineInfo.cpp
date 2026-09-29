#include "Project/Bgm/BgmLineInfo.hpp"

#include <cstring>

#include "Library/Yaml/ByamlIter.hpp"

namespace al {
/**
 * @brief Creates the info of one BGM line (a channel BGMs are played on).
 * @param rIter The yaml iterator of the line entry.
 * @return The new line info.
 */
BgmLineInfo* BgmLineInfo::createInfo(const ByamlIter& rIter) {
    BgmLineInfo* info = new BgmLineInfo();
    rIter.tryGetStringByKey(&info->mName, "Name");
    rIter.tryGetIntByKey(&info->mPriority, "Priority");
    if (!rIter.tryGetBoolByKey(&info->mIsDontChangeLowPriorityLineByAreaChange,
                               "DontChangeLowPriorityLineByAreaChange")) {
        info->mIsDontChangeLowPriorityLineByAreaChange = false;
    }
    if (!rIter.tryGetBoolByKey(&info->mIsDontStopByBgmStopArea, "DontStopByBgmStopArea")) {
        info->mIsDontStopByBgmStopArea = false;
    }
    if (!rIter.tryGetBoolByKey(&info->mIsDontStopByChangeBgmArea, "DontStopByChangeBgmArea")) {
        info->mIsDontStopByChangeBgmArea = false;
    }
    return info;
}

/**
 * @brief Creates the info of a group of BGM lines combined under one name.
 * @param rIter The yaml iterator of the combined line entry.
 * @return The new combined line info.
 */
BgmCombinedLineInfo* BgmCombinedLineInfo::createInfo(const ByamlIter& rIter) {
    BgmCombinedLineInfo* info = new BgmCombinedLineInfo();
    rIter.tryGetStringByKey(&info->mName, "Name");
    ByamlIter lineIter;
    rIter.tryGetIterByKey(&lineIter, "LineInfoList");
    info->mLineInfoList = createInfoList<BgmLineInfo>(lineIter);
    return info;
}

/**
 * @brief Orders two line infos by name.
 * @param pInfoA The first info.
 * @param pInfoB The second info.
 * @return Negative, zero or positive as the first name sorts before, equal to or after the second.
 */
s32 BgmLineInfo::compareInfo(const BgmLineInfo* pInfoA, const BgmLineInfo* pInfoB) {
    return strcmp(pInfoA->mName, pInfoB->mName);
}

/**
 * @brief Orders two combined line infos by name.
 * @param pInfoA The first info.
 * @param pInfoB The second info.
 * @return Negative, zero or positive as the first name sorts before, equal to or after the second.
 */
s32 BgmCombinedLineInfo::compareInfo(const BgmCombinedLineInfo* pInfoA, const BgmCombinedLineInfo* pInfoB) {
    return strcmp(pInfoA->mName, pInfoB->mName);
}
}  // namespace al

#include "Library/Bgm/BgmDataBase.hpp"

#include "Library/Yaml/ByamlIter.hpp"

namespace al {
/**
 * Creates BGM line information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information.
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
 * Creates combined BGM line information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information.
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
 * Compares two BGM line information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 BgmLineInfo::compareInfo(const BgmLineInfo* pA, const BgmLineInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

/**
 * Compares two combined BGM line information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 BgmCombinedLineInfo::compareInfo(const BgmCombinedLineInfo* pA, const BgmCombinedLineInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}
}  // namespace al

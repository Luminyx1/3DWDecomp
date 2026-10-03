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
}  // namespace al

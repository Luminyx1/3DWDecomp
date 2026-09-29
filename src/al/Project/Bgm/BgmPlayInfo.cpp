#include "Project/Bgm/BgmPlayInfo.hpp"

#include <cstring>

#include "Library/Yaml/ByamlIter.hpp"

namespace al {
/**
 * @brief Creates the info describing how a BGM is played.
 * @param rIter The yaml iterator of the play info entry.
 * @return The new play info.
 */
BgmPlayInfo* BgmPlayInfo::createInfo(const ByamlIter& rIter) {
    BgmPlayInfo* info = new BgmPlayInfo();
    rIter.tryGetStringByKey(&info->mName, "Name");
    rIter.tryGetStringByKey(&info->mLineName, "LineName");
    rIter.tryGetStringByKey(&info->mDefaultResourceName, "DefaultResourceName");
    if (!rIter.tryGetBoolByKey(&info->mIsPlayingByUpperLayerAudioUser, "IsPlayingByUpperLayerAudioUser")) {
        info->mIsPlayingByUpperLayerAudioUser = false;
    }
    return info;
}

/**
 * @brief Orders two play infos by name.
 * @param pInfoA The first info.
 * @param pInfoB The second info.
 * @return Negative, zero or positive as the first name sorts before, equal to or after the second.
 */
s32 BgmPlayInfo::compareInfo(const BgmPlayInfo* pInfoA, const BgmPlayInfo* pInfoB) {
    return strcmp(pInfoA->mName, pInfoB->mName);
}
}  // namespace al

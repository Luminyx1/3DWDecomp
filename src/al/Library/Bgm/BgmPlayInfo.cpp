#include "Library/Bgm/BgmDataBase.hpp"

#include "Library/Yaml/ByamlIter.hpp"

namespace al {
/**
 * Creates BGM play information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information.
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
 * Compares two BGM play information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 BgmPlayInfo::compareInfo(const BgmPlayInfo* pA, const BgmPlayInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}
}  // namespace al

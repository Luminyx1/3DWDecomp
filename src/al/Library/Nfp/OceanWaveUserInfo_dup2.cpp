#include "Project/OceanWave/OceanWaveUserInfo.hpp"

#include "Library/Yaml/ByamlIter.hpp"

namespace al {
/**
 * Creates ocean wave play information from BYAML data.
 * @param rIter BYAML data
 * @return created information, or nullptr if the data is invalid
 */
OceanWavePlayInfo* OceanWavePlayInfo::createInfo(const ByamlIter& rIter) {
    OceanWavePlayInfo* info = new OceanWavePlayInfo;
    if (!rIter.tryGetStringByKey(&info->mName, "Name")) {
        return nullptr;
    }

    if (!rIter.tryGetStringByKey(&info->mRequestKeeperName, "RequestKeeperName")) {
        info->mRequestKeeperName = nullptr;
    }

    ByamlIter iter;
    if (!rIter.tryGetIterByKey(&iter, "OceanWaveInfoList")) {
        return nullptr;
    }

    info->mOceanWaveInfoList = createInfoList<OceanWaveInfo>(iter);
    return info;
}

/**
 * Compares two ocean wave play information by name.
 * @param pA first information
 * @param pB second information
 * @return comparison result
 */
s32 OceanWavePlayInfo::compareInfo(const OceanWavePlayInfo* pA, const OceanWavePlayInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}
}  // namespace al

#include "Library/Se/Info/SeAudioInfo.hpp"

#include "Library/Yaml/ByamlIter.hpp"

namespace al {

/**
 * @brief Creates SE action information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information.
 */
SeActionInfo* SeActionInfo::createInfo(const ByamlIter& rIter) {
    SeActionInfo* info = new SeActionInfo;
    rIter.tryGetStringByKey(&info->mName, "Name");

    if (!rIter.tryGetBoolByKey(&info->mIsStopPlayingSe, "IsStopPlayingSe")) {
        info->mIsStopPlayingSe = false;
    }

    ByamlIter playIter;
    rIter.tryGetIterByKey(&playIter, "PlayInfoInActionList");
    info->mPlayInfoList = createInfoList<SePlayInfoInAction>(playIter);
    return info;
}

/**
 * @brief Compares two SE action information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 SeActionInfo::compareInfo(const SeActionInfo* pA, const SeActionInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

} // namespace al

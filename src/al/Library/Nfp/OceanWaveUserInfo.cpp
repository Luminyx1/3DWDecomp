#include "Project/OceanWave/OceanWaveUserInfo.hpp"

#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Base/StringOpUtil.hpp"

namespace al {
namespace {
template <typename T>
AudioInfoList<T>* createEmptyInfoList() {
    AudioInfoList<T>* list = new AudioInfoList<T>;
    list->mNext = nullptr;
    list->mInfos = new sead::PtrArray<T>;
    list->mInfos->allocBuffer(1, nullptr);
    return list;
}
}  // namespace

/**
 * Creates ocean wave user information from BYAML data.
 * @param rIter BYAML data
 * @param rName user name
 * @return created information
 */
OceanWaveUserInfo* OceanWaveUserInfo::createInfo(const ByamlIter& rIter,
                                                 const sead::SafeString& rName) {
    OceanWaveUserInfo* info = new OceanWaveUserInfo;
    info->mName = createStringIfInStack(rName.cstr());
    rIter.tryGetStringByKey(&info->mParentName, "ParentName");
    {
        ByamlIter iter;
        if (rIter.tryGetIterByKey(&iter, "ActionInfoList")) {
            info->mActionInfoList = createInfoList<OceanWaveActionInfo>(iter);
        } else if (info->mParentName != nullptr) {
            info->mActionInfoList = createEmptyInfoList<OceanWaveActionInfo>();
        } else {
            info->mActionInfoList = nullptr;
        }
    }

    {
        ByamlIter iter;
        if (rIter.tryGetIterByKey(&iter, "PlayInfoList")) {
            info->mPlayInfoList = createInfoList<OceanWavePlayInfo>(iter);
        } else if (info->mParentName != nullptr) {
            info->mPlayInfoList = createEmptyInfoList<OceanWavePlayInfo>();
        } else {
            info->mPlayInfoList = nullptr;
        }
    }

    return info;
}

/**
 * Compares two ocean wave user information by name.
 * @param pA first information
 * @param pB second information
 * @return comparison result
 */
s32 OceanWaveUserInfo::compareInfo(const OceanWaveUserInfo* pA, const OceanWaveUserInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}
}  // namespace al

#include "Library/Se/Info/SeAudioInfo.hpp"

#include <cstdio>

#include "Library/Se/Function/SeDbFunction.hpp"
#include "Library/Yaml/ByamlIter.hpp"

namespace al {
/**
 * Creates SE user information from BYAML data.
 * @param rIter BYAML data.
 * @param rName User name.
 * @return Created information.
 */
SeUserInfo* SeUserInfo::createInfo(const ByamlIter& rIter, const sead::SafeString& rName) {
    SeUserInfo* info = new SeUserInfo;
    s32 size = rName.calcLength() + 1;
    char* name = new char[size];
    snprintf(name, size, "%s", rName.cstr());
    info->mName = name;

    {
        ByamlIter actionIter;

        if (rIter.tryGetIterByKey(&actionIter, "ActionInfoList")) {
            info->mActionInfoList = createInfoList<SeActionInfo>(actionIter);
        } else {
            info->mActionInfoList = nullptr;
        }
    }

    {
        ByamlIter playIter;

        if (rIter.tryGetIterByKey(&playIter, "PlayInfoList")) {
            info->mPlayInfoList = createInfoList<SePlayInfo>(playIter);
        } else {
            info->mPlayInfoList = nullptr;
        }
    }

    {
        ByamlIter emitterIter;

        if (rIter.tryGetIterByKey(&emitterIter, "EmitterInfoList")) {
            info->mEmitterInfoList = createInfoList<SeEmitterInfo>(emitterIter);
        } else {
            info->mEmitterInfoList = alSeDbFunction::createDefaultEmitterInfoList();
        }
    }

    return info;
}

/**
 * Compares two SE user information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 SeUserInfo::compareInfo(const SeUserInfo* pA, const SeUserInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

}  // namespace al

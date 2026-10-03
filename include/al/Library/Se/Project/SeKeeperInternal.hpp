#pragma once

#include <attributes.h>

#include "Library/Se/DataBase/SeDataBase.hpp"
#include "Library/Se/Info/SeAudioInfo.hpp"

namespace al {
namespace detail {
using SpecificInfoList = AudioInfoList<SeResourceSpecificInfo>;
extern HIDDEN SeResourceSpecificInfo sDefaultSpecificInfo;

/**
 * @brief Finds resource-specific settings by sound identifier, falling back to shared defaults.
 * @param pDataBase Non-null database containing the resource list.
 * @param id Sound identifier; the invalid identifier skips the search.
 * @return Matching resource settings or the shared default settings.
 */
inline const al::SeResourceSpecificInfo* findSpecificInfo(const al::SeDataBase* pDataBase, u32 id) {
    const al::SeResourceSpecificInfo* pInfo = nullptr;
    const SpecificInfoList* pList = pDataBase->getResourceSpecificInfoList();

    if (pList != nullptr && al::AudioConst::SOUND_ID_INVALID != id) {
        do {
            for (s32 i = 0; i < pList->mInfos->size(); i++) {
                if (pList->mInfos->unsafeAt(i)->mSoundId == id) {
                    pInfo = pList->mInfos->unsafeAt(i);
                    goto found;
                }
            }

            pList = pList->mNext;
        } while (pList != nullptr);
    }

found:
    return pInfo != nullptr ? pInfo : &sDefaultSpecificInfo;
}
} // namespace detail
} // namespace al

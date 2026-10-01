#include "Library/Effect/EmitterSetResourceInfoHolder.hpp"

#include <cstring>

#include "Library/Effect/PtclSystem.hpp"

namespace al {

static inline bool isExistEmitterSet(const PtclSystem* pPtclSystem, s32 resourceNum,
                                     const char* pName) {
    for (s32 i = 0; i < resourceNum; i++) {
        if (pPtclSystem->GetResource(i)->SearchEmitterSetId(pName) >= 0) {
            return true;
        }
    }

    return false;
}

/**
 * Constructs the info of one emitter set in a particle resource.
 * @param pName Emitter set name.
 * @param resourceId Index of the particle resource.
 * @param emitterSetId Index of the emitter set in the resource.
 * @param flags Flags.
 * @param isLoop Whether the emitter set loops.
 * @param isInfinity Whether the emitter set has an infinite life.
 */
EmitterSetResourceInfo::EmitterSetResourceInfo(const char* pName, s32 resourceId,
                                               s32 emitterSetId, u32 flags, bool isLoop,
                                               bool isInfinity)
    : mName(pName), mResourceId(resourceId), mEmitterSetId(emitterSetId), mFlags(flags),
      mIsLoop(isLoop), mIsInfinity(isInfinity) {}

void EmitterSetResourceInfoHolder::createDataBase(PtclSystem* pPtclSystem) {
    mInfos.clear();

    s32 emitterSetNum = 0;
    for (s32 i = 0; i < pPtclSystem->getNumResource(); i++) {
        nn::vfx::Resource* resource = pPtclSystem->GetResource(i);

        if (resource != nullptr) {
            emitterSetNum += resource->GetEmitterSetNum();
        }
    }

    if (mInfos.capacity() < emitterSetNum) {
        mInfos.tryAllocBuffer(emitterSetNum + 0x40, nullptr, 8);
    }

    for (s32 i = 0; i < pPtclSystem->getNumResource(); i++) {
        nn::vfx::Resource* resource = pPtclSystem->GetResource(i);

        if (resource == nullptr) {
            continue;
        }

        for (s32 j = 0; j < resource->GetEmitterSetNum(); j++) {
            const char* name = resource->GetEmitterSetName(j);

            if (isExistEmitterSet(pPtclSystem, i, name)) {
                continue;
            }

            bool isLoop = false;
            bool isInfinity = false;
            nn::vfx::Resource* ownResource = pPtclSystem->GetResource(i);

            if (j < ownResource->GetEmitterSetNum()) {
                nn::vfx::EmitterSetResource* emitterSetResource =
                    ownResource->GetEmitterSetResource(j);

                if (emitterSetResource != nullptr) {
                    isLoop = emitterSetResource->m_IsLoop;
                    isInfinity = emitterSetResource->m_IsInfinity;
                }
            }

            mInfos.emplaceBack(name, i, j, 0, isLoop, isInfinity);
        }
    }

    mInfos.sort_<EmitterSetResourceInfo>(
        [](const EmitterSetResourceInfo* pA, const EmitterSetResourceInfo* pB) -> s32 {
            return strcmp(pA->mName, pB->mName);
        });
}

inline s32 EmitterSetResourceInfoHolder::searchIndex(const char* pName) const {
    if (mInfos.size() == 0) {
        return -1;
    }

    s32 low = 0;
    s32 high = mInfos.size() - 1;
    while (low < high) {
        s32 mid = (low + high) / 2;
        s32 result = strcmp(mInfos.unsafeAt(mid)->mName, pName);

        if (result == 0) {
            return mid;
        }

        if (result < 0) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }

    if (strcmp(mInfos.unsafeAt(low)->mName, pName) == 0) {
        return low;
    }

    return -1;
}

/**
 * Finds the resource info of an emitter set by name.
 * @param pName Emitter set name.
 * @return The info, or the invalid resource info if not found.
 */
EmitterSetResourceInfo* EmitterSetResourceInfoHolder::tryFindEffectResouceInfo(
    const char* pName) const {
    s32 index = searchIndex(pName);

    if (index < 0) {
        return &EmitterSetResourceInfo::InvalidResource;
    }

    return mInfos.at(index);
}

/**
 * Finds the resource info of an emitter set by name.
 * @param pName Emitter set name.
 * @return The info, or the invalid resource info if not found.
 */
EmitterSetResourceInfo* EmitterSetResourceInfoHolder::findEffectResouceInfo(
    const char* pName) const {
    s32 index = searchIndex(pName);
    if (index >= 0) {
        return mInfos.at(index);
    }

    return &EmitterSetResourceInfo::InvalidResource;
}

}  // namespace al

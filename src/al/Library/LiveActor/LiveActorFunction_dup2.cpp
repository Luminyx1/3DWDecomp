#include "Library/LiveActor/SubActorUtil.hpp"

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/SubActorKeeper.hpp"
#include "Library/Model/ModelKeeper.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
namespace {
inline LiveActor* findSubActorInline(const SubActorKeeper* pKeeper, const char* pName) {
    for (s32 i = 0; i < pKeeper->mCount; i++) {
        const SubActorInfo* info = pKeeper->mInfos[i];
        if (isEqualString(info->mSubActor->getName(), pName)) {
            return info->mSubActor;
        }
    }
    return nullptr;
}

inline SubActorInfo* getSubActorInfo(const LiveActor* pActor, const LiveActor* pSubActor) {
    SubActorKeeper* keeper = pActor->mSubActorKeeper;
    for (s32 i = 0; i < keeper->mCount; i++) {
        SubActorInfo* info = keeper->mInfos[i];
        if (info->mSubActor == pSubActor) {
            return info;
        }
    }
    return nullptr;
}
}  // namespace

/**
 * Checks whether an actor has a sub actor keeper.
 * @param pActor The actor.
 * @return Whether the sub actor keeper exists.
 */
bool isExistSubActorKeeper(const LiveActor* pActor) {
    return pActor->mSubActorKeeper != nullptr;
}

/**
 * Copies an actor's global alpha to all of its sub actors.
 * @param pActor The actor.
 */
void setSubActorAlpha(LiveActor* pActor) {
    SubActorKeeper* keeper = pActor->mSubActorKeeper;
    if (!keeper) {
        return;
    }
    for (s32 i = 0; i < keeper->mCount; i++) {
        LiveActor* subActor = keeper->mInfos[i]->mSubActor;
        subActor->mGlobalAlphaLastFrame = pActor->mGlobalAlphaLastFrame;
    }
}

/**
 * Sets the global alpha pointer of all sub actor models.
 * @param pActor The actor.
 * @param pAlpha The alpha pointer.
 */
void setSubActorAlphaPtr(LiveActor* pActor, f32* pAlpha) {
    SubActorKeeper* keeper = pActor->mSubActorKeeper;
    if (!keeper) {
        return;
    }
    for (s32 i = 0; i < keeper->mCount; i++) {
        LiveActor* subActor = keeper->mInfos[i]->mSubActor;
        if (subActor && subActor->mModelKeeper) {
            subActor->mModelKeeper->setGlobalAlpha(pAlpha);
        }
    }
}

/**
 * Gets a sub actor by name.
 * @param pActor The actor.
 * @param pName The sub actor name.
 * @return The sub actor or nullptr.
 */
LiveActor* getSubActor(const LiveActor* pActor, const char* pName) {
    if (!isExistSubActorKeeper(pActor)) {
        return nullptr;
    }
    return findSubActorInline(pActor->mSubActorKeeper, pName);
}

/**
 * Gets a sub actor by name if it exists.
 * @param pActor The actor.
 * @param pName The sub actor name.
 * @return The sub actor or nullptr.
 */
LiveActor* tryGetSubActor(const LiveActor* pActor, const char* pName) {
    return getSubActor(pActor, pName);
}

/**
 * Gets a sub actor by index.
 * @param pActor The actor.
 * @param index The index.
 * @return The sub actor.
 */
LiveActor* getSubActor(const LiveActor* pActor, s32 index) {
    return pActor->mSubActorKeeper->mInfos[index]->mSubActor;
}

/**
 * Gets the number of sub actors.
 * @param pActor The actor.
 * @return The sub actor count.
 */
s32 getSubActorNum(const LiveActor* pActor) {
    return pActor->mSubActorKeeper->mCount;
}

/**
 * Stops a sub actor from syncing its clipping with the actor.
 * @param pActor The actor.
 * @param pSubActor The sub actor.
 */
void offSyncClippingSubActor(LiveActor* pActor, const LiveActor* pSubActor) {
    SubActorInfo* info = getSubActorInfo(pActor, pSubActor);
    if (info->mSyncType & 2) {
        info->mSyncType &= ~2;
    }
}

/**
 * Stops all sub actors from syncing their clipping with the actor.
 * @param pActor The actor.
 */
void offSyncClippingSubActorAll(LiveActor* pActor) {
    SubActorKeeper* keeper = pActor->mSubActorKeeper;
    for (s32 i = 0; i < keeper->mCount; i++) {
        SubActorInfo* info = keeper->mInfos[i];
        if (info->mSyncType & 2) {
            info->mSyncType &= ~2;
        }
    }
}

/**
 * Makes a sub actor sync its clipping with the actor.
 * @param pActor The actor.
 * @param pSubActor The sub actor.
 */
void onSyncClippingSubActor(LiveActor* pActor, const LiveActor* pSubActor) {
    SubActorInfo** infos = pActor->mSubActorKeeper->mInfos;
    SubActorInfo* info;
    do {
        info = *infos++;
    } while (info->mSubActor != pSubActor);
    info->mSyncType |= 2;
}

/**
 * Makes all sub actors sync their clipping with the actor.
 * @param pActor The actor.
 */
void onSyncClippingSubActorAll(LiveActor* pActor) {
    SubActorKeeper* keeper = pActor->mSubActorKeeper;
    for (s32 i = 0; i < keeper->mCount; i++) {
        keeper->mInfos[i]->mSyncType |= 2;
    }
}
}  // namespace al

#include "Library/LiveActor/LiveActorFunc.hpp"

#include "Project/Base/StringUtil.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/SubActorKeeper.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"

namespace alSubActorFunction {
/**
 * Makes all sub actors that sync appearance appear.
 * @param pKeeper The sub actor keeper.
 */
void trySyncAlive(al::SubActorKeeper* pKeeper) {
    for (s32 i = 0; i < pKeeper->mCount; i++) {
        if (pKeeper->mInfos[i]->mSyncType & 1) {
            pKeeper->mInfos[i]->mSubActor->makeActorAppeared();
        }
    }
}

/**
 * Makes all sub actors that sync appearance dead.
 * @param pKeeper The sub actor keeper.
 */
void trySyncDead(al::SubActorKeeper* pKeeper) {
    for (s32 i = 0; i < pKeeper->mCount; i++) {
        if (pKeeper->mInfos[i]->mSyncType & 1) {
            pKeeper->mInfos[i]->mSubActor->makeActorDead();
        }
    }
}

/**
 * Starts clipping all alive, unclipped sub actors that sync clipping.
 * @param pKeeper The sub actor keeper.
 */
void trySyncClippingStart(al::SubActorKeeper* pKeeper) {
    for (s32 i = 0; i < pKeeper->mCount; i++) {
        const al::SubActorInfo* info = pKeeper->mInfos[i];
        if ((info->mSyncType & 2) && al::isAlive(info->mSubActor) &&
            !al::isClipped(info->mSubActor)) {
            info->mSubActor->startClipped();
        }
    }
}

/**
 * Ends clipping all alive, clipped sub actors that sync clipping.
 * @param pKeeper The sub actor keeper.
 */
void trySyncClippingEnd(al::SubActorKeeper* pKeeper) {
    for (s32 i = 0; i < pKeeper->mCount; i++) {
        const al::SubActorInfo* info = pKeeper->mInfos[i];
        if ((info->mSyncType & 2) && al::isAlive(info->mSubActor) &&
            al::isClipped(info->mSubActor)) {
            info->mSubActor->endClipped();
        }
    }
}

/**
 * Shows the models of all sub actors that sync hiding.
 * @param pKeeper The sub actor keeper.
 */
void trySyncShowModel(al::SubActorKeeper* pKeeper) {
    for (s32 i = 0; i < pKeeper->mCount; i++) {
        if (pKeeper->mInfos[i]->mSyncType & 4) {
            al::showModelIfHide(pKeeper->mInfos[i]->mSubActor);
        }
    }
}

/**
 * Hides the models of all sub actors that sync hiding.
 * @param pKeeper The sub actor keeper.
 */
void trySyncHideModel(al::SubActorKeeper* pKeeper) {
    for (s32 i = 0; i < pKeeper->mCount; i++) {
        if (pKeeper->mInfos[i]->mSyncType & 4) {
            al::hideModelIfShow(pKeeper->mInfos[i]->mSubActor);
        }
    }
}

/**
 * Finds a sub actor by name.
 * @param pKeeper The sub actor keeper.
 * @param pName The sub actor name.
 * @return The sub actor or nullptr.
 */
al::LiveActor* findSubActor(const al::SubActorKeeper* pKeeper, const char* pName) {
    for (s32 i = 0; i < pKeeper->mCount; i++) {
        const al::SubActorInfo* info = pKeeper->mInfos[i];
        if (al::isEqualString(info->mSubActor->getName(), pName)) {
            return info->mSubActor;
        }
    }

    return nullptr;
}

/**
 * Sets the global Y offset reference of all sub actors.
 * @param pKeeper The sub actor keeper.
 * @param pOffset The Y offset reference.
 */
void setGlobalYOffset(al::SubActorKeeper* pKeeper, f32* pOffset) {
    for (s32 i = 0; i < pKeeper->mCount; i++) {
        pKeeper->mInfos[i]->mSubActor->setGlobalYOffsetRef(pOffset);
    }
}
}  // namespace alSubActorFunction

namespace al {
/**
 * Constructs an empty sub actor info.
 */
SubActorInfo::SubActorInfo() = default;
}  // namespace al

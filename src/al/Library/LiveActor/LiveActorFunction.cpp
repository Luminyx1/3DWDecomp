#include "Library/LiveActor/LiveActorFunc.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/SubActorKeeper.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace alSubActorFunction {
    /**
     * @brief Makes every sub actor that syncs appearance appear.
     * @param pKeeper The sub actors of the actor that appeared.
     */
    void trySyncAlive(al::SubActorKeeper* pKeeper) {
        for (s32 i = 0; i < pKeeper->mCurActorCount; i++) {
            al::SubActorInfo* pInfo = pKeeper->mActorInfos[i];

            if (pInfo->mSyncType.isOn(al::SubActorSync::cAppear)) {
                pInfo->mSubActor->makeActorAppeared();
            }
        }
    }

    /**
     * @brief Kills every sub actor that syncs appearance.
     * @param pKeeper The sub actors of the actor that died.
     */
    void trySyncDead(al::SubActorKeeper* pKeeper) {
        for (s32 i = 0; i < pKeeper->mCurActorCount; i++) {
            al::SubActorInfo* pInfo = pKeeper->mActorInfos[i];

            if (pInfo->mSyncType.isOn(al::SubActorSync::cAppear)) {
                pInfo->mSubActor->makeActorDead();
            }
        }
    }

    /**
     * @brief Clips every alive, unclipped sub actor that syncs clipping.
     * @param pKeeper The sub actors of the actor that was clipped.
     */
    void trySyncClippingStart(al::SubActorKeeper* pKeeper) {
        for (s32 i = 0; i < pKeeper->mCurActorCount; i++) {
            al::SubActorInfo* pInfo = pKeeper->mActorInfos[i];

            if (pInfo->mSyncType.isOn(al::SubActorSync::cClipping) && al::isAlive(pInfo->mSubActor) &&
                !al::isClipped(pInfo->mSubActor)) {
                pInfo->mSubActor->startClipped();
            }
        }
    }

    /**
     * @brief Unclips every alive, clipped sub actor that syncs clipping.
     * @param pKeeper The sub actors of the actor that was unclipped.
     */
    void trySyncClippingEnd(al::SubActorKeeper* pKeeper) {
        for (s32 i = 0; i < pKeeper->mCurActorCount; i++) {
            al::SubActorInfo* pInfo = pKeeper->mActorInfos[i];

            if (pInfo->mSyncType.isOn(al::SubActorSync::cClipping) && al::isAlive(pInfo->mSubActor) &&
                al::isClipped(pInfo->mSubActor)) {
                pInfo->mSubActor->endClipped();
            }
        }
    }

    /**
     * @brief Shows the model of every sub actor that syncs visibility.
     * @param pKeeper The sub actors of the actor that was shown.
     */
    void trySyncShowModel(al::SubActorKeeper* pKeeper) {
        for (s32 i = 0; i < pKeeper->mCurActorCount; i++) {
            al::SubActorInfo* pInfo = pKeeper->mActorInfos[i];

            if (pInfo->mSyncType.isOn(al::SubActorSync::cHide)) {
                al::showModelIfHide(pInfo->mSubActor);
            }
        }
    }

    /**
     * @brief Hides the model of every sub actor that syncs visibility.
     * @param pKeeper The sub actors of the actor that was hidden.
     */
    void trySyncHideModel(al::SubActorKeeper* pKeeper) {
        for (s32 i = 0; i < pKeeper->mCurActorCount; i++) {
            al::SubActorInfo* pInfo = pKeeper->mActorInfos[i];

            if (pInfo->mSyncType.isOn(al::SubActorSync::cHide)) {
                al::hideModelIfShow(pInfo->mSubActor);
            }
        }
    }

    /**
     * @brief Finds a sub actor by name.
     * @param pKeeper The sub actors to search.
     * @param pName The name of the sub actor.
     * @return The sub actor, or nullptr if none has that name.
     */
    al::LiveActor* findSubActor(const al::SubActorKeeper* pKeeper, const char* pName) {
        for (s32 i = 0; i < pKeeper->mCurActorCount; i++) {
            al::SubActorInfo* pInfo = pKeeper->mActorInfos[i];

            if (al::isEqualString(pInfo->mSubActor->getName(), pName)) {
                return pInfo->mSubActor;
            }
        }

        return nullptr;
    }

    /**
     * @brief Links the Y offset of every sub actor to a value.
     * @param pKeeper The sub actors to update.
     * @param pOffset The Y offset to follow.
     */
    void setGlobalYOffset(al::SubActorKeeper* pKeeper, f32* pOffset) {
        for (s32 i = 0; i < pKeeper->mCurActorCount; i++) {
            pKeeper->mActorInfos[i]->mSubActor->setGlobalYOffsetRef(pOffset);
        }
    }
};

namespace al {
    /** @brief Creates an empty sub actor entry. */
    SubActorInfo::SubActorInfo() {}
};

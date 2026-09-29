#include "Library/Item/ActorItemKeeper.hpp"

#include "Library/Item/ItemDirectorBase.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"

namespace al {
    /**
     * @brief Constructs an item keeper with room for a fixed number of item infos.
     * @param pActor The actor owning the items.
     * @param maxItems The maximum number of item infos that can be added.
     */
    ActorItemKeeper::ActorItemKeeper(const LiveActor* pActor, s32 maxItems)
        : mHostActor(pActor), mMaxItemInfos(maxItems) {
        mItemOffset.x = 0;
        mItemOffset.y = 0;
        mItemOffset.z = 0;
        mItemInfos = new ActorItemInfo*[mMaxItemInfos];
    }

    /**
     * @brief Declares an item to the scene's item director and registers a new item info.
     * @param rInfo The actor init info used to declare the item.
     * @param pItemName The kind of item to appear.
     * @param pTiming The timing name the item appears at.
     * @param pFactor The factor name the item appears for.
     * @param isUnk Unused flag.
     * @return The newly created item info.
     */
    ActorItemInfo* ActorItemKeeper::addItem(const ActorInitInfo& rInfo, const char* pItemName,
                                            const char* pTiming, const char* pFactor, bool isUnk) {
        if (mHostActor->getSceneInfo()->itemDirectorBase != nullptr) {
            ActorSceneInfo* sceneInfo = mHostActor->getSceneInfo();
            sceneInfo->itemDirectorBase->declareItem(pItemName, rInfo);
        }

        ActorItemInfo* info = new ActorItemInfo(pItemName, pTiming, pFactor);
        mItemInfos[mCurNumItemInfos] = info;
        mCurNumItemInfos++;
        return info;
    }

    /**
     * @brief Sets the factor and attacker sensor used when making items appear.
     * @param pFactor The factor name.
     * @param pSensor The attacker sensor.
     */
    void ActorItemKeeper::setFactor(const char* pFactor, const HitSensor* pSensor) {
        mFactor = pFactor;
        mAttackerSensor = pSensor;
    }

    /**
     * @brief Finds the item info matching a timing and the current factor.
     * @param pTiming The timing name to search for.
     * @return The matching item info, or nullptr if none matches.
     */
    ActorItemInfo* ActorItemKeeper::getAppearItemInfo(const char* pTiming) const {
        for (s32 i = 0; i < mCurNumItemInfos; i++) {
            if (mItemInfos[i]->isEqualTiming(pTiming) && mItemInfos[i]->isEqualFactor(mFactor)) {
                return mItemInfos[i];
            }
        }

        return nullptr;
    }
}  // namespace al

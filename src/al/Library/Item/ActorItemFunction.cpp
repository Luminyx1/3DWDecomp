#include "Library/Item/AcquireItemFunc.hpp"

#include "Library/Item/ActorItemKeeper.hpp"
#include "Library/LiveActor/LiveActor.hpp"

namespace al {
    /**
     * @brief Adds an item to the actor's item keeper.
     * @param pActor The actor owning the item keeper.
     * @param rInfo The actor init info used to declare the item.
     * @param pItemName The kind of item to appear.
     * @param pTiming The timing name the item appears at.
     * @param pFactor The factor name the item appears for.
     * @param isUnk Flag forwarded to the item keeper.
     * @return The newly created item info.
     */
    ActorItemInfo* addItem(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pItemName,
                           const char* pTiming, const char* pFactor, bool isUnk) {
        return pActor->mItemKeeper->addItem(rInfo, pItemName, pTiming, pFactor, isUnk);
    }
}  // namespace al

#include "Library/Item/ItemUtil.hpp"

#include "Library/Item/ActorItemKeeper.hpp"
#include "Library/Item/ItemDirectorBase.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"

namespace al {
    /**
     * @brief Adds an item without timing or factor to the actor's item keeper.
     * @param pActor The actor owning the item keeper.
     * @param rInfo The actor init info used to declare the item.
     * @param pItemName The kind of item to appear.
     * @param isUnk Flag forwarded to the item keeper.
     * @return The newly created item info.
     */
    ActorItemInfo* addItem(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pItemName, bool isUnk) {
        return pActor->getActorItemKeeper()->addItem(rInfo, pItemName, nullptr, nullptr, isUnk);
    }

    /**
     * @brief Sets the factor and attacker sensor used for the actor's next item appearance.
     * @param pActor The actor owning the item keeper.
     * @param pFactor The factor name.
     * @param pSensor The attacker sensor.
     */
    void setAppearItemFactor(const LiveActor* pActor, const char* pFactor, const HitSensor* pSensor) {
        pActor->getActorItemKeeper()->setFactor(pFactor, pSensor);
    }

    /**
     * @brief Sets the offset added to the appear position of the actor's next item.
     * @param pActor The actor owning the item keeper.
     * @param rOffset The offset to add.
     */
    void setAppearItemOffset(const LiveActor* pActor, const sead::Vector3f& rOffset) {
        static_cast<sead::BaseVec3<f32>&>(pActor->getActorItemKeeper()->mItemOffset) = rOffset;
    }

    /**
     * @brief Sets the attacker sensor used for the actor's next item appearance.
     * @param pActor The actor owning the item keeper.
     * @param pSensor The attacker sensor.
     */
    void setAppearItemAttackerSensor(const LiveActor* pActor, const HitSensor* pSensor) {
        pActor->getActorItemKeeper()->setAttackerSensor(pSensor);
    }

    /**
     * @brief Makes the actor's default item appear at its position.
     * @param pActor The actor owning the item keeper.
     */
    void appearItem(const LiveActor* pActor) {
        appearItemTiming(pActor, nullptr, getTrans(pActor), sead::Vector3f::ez,
                         pActor->getActorItemKeeper()->getAttackerSensor(), false);
    }

    /**
     * @brief Makes the actor's default item appear at a given position.
     * @param pActor The actor owning the item keeper.
     * @param rPos The appear position.
     * @param rFront The appear front direction.
     * @param pSensor The attacker sensor.
     */
    void appearItem(const LiveActor* pActor, const sead::Vector3f& rPos, const sead::Vector3f& rFront,
                    const HitSensor* pSensor) {
        appearItemTiming(pActor, nullptr, rPos, rFront, pSensor, false);
    }

    /**
     * @brief Makes the actor's default item appear at a given position using the stored attacker sensor.
     * @param pActor The actor owning the item keeper.
     * @param rPos The appear position.
     * @param rFront The appear front direction.
     */
    void appearItem(const LiveActor* pActor, const sead::Vector3f& rPos, const sead::Vector3f& rFront) {
        appearItemTiming(pActor, nullptr, rPos, rFront, pActor->getActorItemKeeper()->getAttackerSensor(), false);
    }

    /**
     * @brief Makes the actor's item for a timing appear at its position.
     * @param pActor The actor owning the item keeper.
     * @param pTiming The timing name.
     */
    void appearItemTiming(const LiveActor* pActor, const char* pTiming) {
        appearItemTiming(pActor, pTiming, getTrans(pActor), sead::Vector3f::ez,
                         pActor->getActorItemKeeper()->getAttackerSensor(), false);
    }

    /**
     * @brief Makes the actor's item for a timing appear, then resets the factor and offset.
     * @param pActor The actor owning the item keeper.
     * @param pTiming The timing name.
     * @param rPos The appear position.
     * @param rFront The appear front direction.
     * @param pSensor The attacker sensor.
     * @param isUnk Flag forwarded to the item director.
     */
    void appearItemTiming(const LiveActor* pActor, const char* pTiming, const sead::Vector3f& rPos,
                          const sead::Vector3f& rFront, const HitSensor* pSensor, bool isUnk) {
        ActorItemInfo* info = pActor->getActorItemKeeper()->getAppearItemInfo(pTiming);

        if (info == nullptr) {
            return;
        }

        const char* itemKind = info->mItemKind;
        ItemDirectorBase* director = pActor->getSceneInfo()->itemDirectorBase;
        ActorItemKeeper* keeper = pActor->getActorItemKeeper();
        sead::Vector3f pos = rPos + keeper->mItemOffset;
        director->appearItem(itemKind, pos, rFront, pSensor, isUnk, false);
        keeper->mFactor = nullptr;
        static_cast<sead::BaseVec3<f32>&>(keeper->mItemOffset) = sead::Vector3f::zero;
    }

    /**
     * @brief Makes the actor's item for a timing appear using the stored attacker sensor.
     * @param pActor The actor owning the item keeper.
     * @param pTiming The timing name.
     * @param rPos The appear position.
     * @param rFront The appear front direction.
     */
    void appearItemTiming(const LiveActor* pActor, const char* pTiming, const sead::Vector3f& rPos,
                          const sead::Vector3f& rFront) {
        appearItemTiming(pActor, pTiming, rPos, rFront, pActor->getActorItemKeeper()->getAttackerSensor(), false);
    }

    /**
     * @brief Notifies the item director that an item was acquired.
     * @param pActor The acquired item actor.
     * @param pSensor The sensor of the acquirer.
     * @param pName The item name.
     */
    void acquirerItem(const LiveActor* pActor, HitSensor* pSensor, const char* pName) {
        pActor->getSceneInfo()->itemDirectorBase->acquirerItem(pActor, pSensor, pName);
    }
}  // namespace al

#include "Library/MapObj/SupportFreezeSyncGroupHolder.hpp"

#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementId.hpp"

namespace al {
    /**
     * @brief Constructs an empty support freeze sync group.
     */
    SupportFreezeSyncGroup::SupportFreezeSyncGroup() : mGroupId(new PlacementId()) {}

    /**
     * @brief Reads the group id and allocates the actor list.
     * @param rInfo The actor init info holding the group link.
     */
    void SupportFreezeSyncGroup::init(const ActorInitInfo& rInfo) {
        alPlacementFunction::getLinkGroupId(mGroupId, rInfo, "SupportFreezeSyncGroup");
        mActors = new LiveActor*[mActorMaxNum];
        for (s32 i = 0; i < mActorMaxNum; i++) {
            mActors[i] = nullptr;
        }
    }

    /**
     * @brief Adds an actor to the group if there is room.
     * @param pActor The actor to add.
     */
    void SupportFreezeSyncGroup::regist(LiveActor* pActor) {
        if (mActorNum >= mActorMaxNum) {
            return;
        }
        mActors[mActorNum] = pActor;
        mActorNum++;
    }

    /**
     * @brief Sets the sensor used to send the sync messages.
     * @param pSensor The host sensor.
     */
    void SupportFreezeSyncGroup::setHostSensor(HitSensor* pSensor) {
        mHostSensor = pSensor;
    }

    /**
     * @brief Checks whether an actor's group link matches this group.
     * @param rInfo The actor init info holding the group link.
     * @return Whether the group ids are equal.
     */
    bool SupportFreezeSyncGroup::isEqualGroupId(const ActorInitInfo& rInfo) const {
        if (!mGroupId->isValid()) {
            return false;
        }

        PlacementId groupId;
        if (!alPlacementFunction::getLinkGroupId(&groupId, rInfo, "SupportFreezeSyncGroup")) {
            return false;
        }
        return mGroupId->isEqual(groupId);
    }

    /**
     * @brief Freezes every actor of the group while any of them is in its support freeze nerve.
     */
    void SupportFreezeSyncGroup::movement() {
        bool isAnyFreeze = false;
        for (s32 i = 0; i < mActorNum; i++) {
            isAnyFreeze |= sendMsgIsNerveSupportFreeze(getHitSensor(mActors[i], 0), mHostSensor);
        }

        for (s32 i = 0; i < mActorNum; i++) {
            if (isAnyFreeze) {
                sendMsgOnSyncSupportFreeze(getHitSensor(mActors[i], 0), mHostSensor);
            } else {
                sendMsgOffSyncSupportFreeze(getHitSensor(mActors[i], 0), mHostSensor);
            }
        }
    }
}  // namespace al

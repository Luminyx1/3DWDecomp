#include "Library/MapObj/SupportFreezeSyncGroupHolder.hpp"

#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementId.hpp"

namespace al {
/**
 * Constructs a support freeze sync group.
 */
SupportFreezeSyncGroup::SupportFreezeSyncGroup() : mPlacementId(new PlacementId()) {}

/**
 * Reads the group id and allocates the actor list.
 * @param rInfo actor init info
 */
void SupportFreezeSyncGroup::init(const ActorInitInfo& rInfo) {
    alPlacementFunction::getLinkGroupId(mPlacementId, rInfo, "SupportFreezeSyncGroup");
    mActors = new LiveActor*[mActorMax];

    for (s32 i = 0; i < mActorMax; i++) {
        mActors[i] = nullptr;
    }
}

/**
 * Adds an actor to the group.
 * @param pActor actor
 */
void SupportFreezeSyncGroup::regist(LiveActor* pActor) {
    if (mActorNum < mActorMax) {
        mActors[mActorNum] = pActor;
        mActorNum++;
    }
}

/**
 * Sets the sensor that sends the sync messages.
 * @param pSensor host sensor
 */
void SupportFreezeSyncGroup::setHostSensor(HitSensor* pSensor) {
    mHostSensor = pSensor;
}

/**
 * Checks if an actor belongs to this group.
 * @param rInfo actor init info
 * @return whether the link group ids are equal
 */
bool SupportFreezeSyncGroup::isEqualGroupId(const ActorInitInfo& rInfo) const {
    if (mPlacementId->mPlacementID == nullptr) {
        return false;
    }

    PlacementId id;

    if (!alPlacementFunction::getLinkGroupId(&id, rInfo, "SupportFreezeSyncGroup")) {
        return false;
    }

    return mPlacementId->isEqual(id);
}

/**
 * Freezes all actors of the group while any of them is support frozen.
 */
void SupportFreezeSyncGroup::movement() {
    bool isFreeze = false;

    for (s32 i = 0; i < mActorNum; i++) {
        isFreeze |= sendMsgIsNerveSupportFreeze(getHitSensor(mActors[i], 0), mHostSensor);
    }

    if (mActorNum <= 0) {
        return;
    }

    if (isFreeze) {
        for (s32 i = 0; i < mActorNum; i++) {
            sendMsgOnSyncSupportFreeze(getHitSensor(mActors[i], 0), mHostSensor);
        }
    } else {
        for (s32 i = 0; i < mActorNum; i++) {
            sendMsgOffSyncSupportFreeze(getHitSensor(mActors[i], 0), mHostSensor);
        }
    }
}
}  // namespace al

#include "Library/MapObj/SupportFreezeSyncGroupHolder.hpp"

#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"

namespace al {
/**
 * Constructs the support freeze sync group holder.
 */
SupportFreezeSyncGroupHolder::SupportFreezeSyncGroupHolder() : LiveActor("DRC拘束グループ監視") {
    mGroups = new SupportFreezeSyncGroup*[mGroupMax];

    for (s32 i = 0; i < mGroupMax; i++) {
        mGroups[i] = nullptr;
    }
}

/**
 * Initializes the holder and gives the groups its sensor.
 * @param rInfo actor init info
 */
void SupportFreezeSyncGroupHolder::initAfterPlacementSceneObj(const ActorInitInfo& rInfo) {
    initActorSceneInfo(this, rInfo);
    initExecutorUpdate(this, rInfo, "DRCアシスト同期グループ");
    initActorPoseTRSV(this);
    initHitSensor(1);
    HitSensor* sensor = addHitSensorMapObj(this, rInfo, "Body", 0.0f, 0, {0.0f, 0.0f, 0.0f});

    for (s32 i = 0; i < mGroupNum; i++) {
        mGroups[i]->setHostSensor(sensor);
    }

    makeActorAppeared();
}

/**
 * Updates all groups.
 */
void SupportFreezeSyncGroupHolder::movement() {
    for (s32 i = 0; i < mGroupNum; i++) {
        mGroups[i]->movement();
    }
}

/**
 * Registers an actor to the group of its link group id.
 * @param pActor actor
 * @param rInfo actor init info
 */
void SupportFreezeSyncGroupHolder::regist(LiveActor* pActor, const ActorInitInfo& rInfo) {
    SupportFreezeSyncGroup* group = tryFindGroup(rInfo);

    if (!group) {
        group = new SupportFreezeSyncGroup();
        group->init(rInfo);
        mGroups[mGroupNum] = group;
        mGroupNum++;
    }

    group->regist(pActor);
}

/**
 * Finds the group of a link group id.
 * @param rInfo actor init info
 * @return group, or null
 */
SupportFreezeSyncGroup* SupportFreezeSyncGroupHolder::tryFindGroup(const ActorInitInfo& rInfo) {
    for (s32 i = 0; i < mGroupNum; i++) {
        if (mGroups[i]->isEqualGroupId(rInfo)) {
            return mGroups[i];
        }
    }

    return nullptr;
}
}  // namespace al

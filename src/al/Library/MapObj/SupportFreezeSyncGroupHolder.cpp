#include "Library/MapObj/SupportFreezeSyncGroupHolder.hpp"

#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"

namespace al {
    /**
     * @brief Constructs the scene object that keeps all support freeze sync groups.
     */
    SupportFreezeSyncGroupHolder::SupportFreezeSyncGroupHolder() : LiveActor("DRC拘束グループ監視") {
        mGroups = new SupportFreezeSyncGroup*[mGroupMaxNum];
        for (s32 i = 0; i < mGroupMaxNum; i++) {
            mGroups[i] = nullptr;
        }
    }

    /**
     * @brief Initializes the holder actor and gives every group its host sensor.
     * @param rInfo The actor init info.
     */
    void SupportFreezeSyncGroupHolder::initAfterPlacementSceneObj(const ActorInitInfo& rInfo) {
        initActorSceneInfo(this, rInfo);
        initExecutorUpdate(this, rInfo, "DRCアシスト同期グループ");
        initActorPoseTRSV(this);
        initHitSensor(1);

        HitSensor* bodySensor = addHitSensorMapObj(this, rInfo, "Body", 0.0f, 0, {0.0f, 0.0f, 0.0f});
        for (s32 i = 0; i < mGroupNum; i++) {
            mGroups[i]->setHostSensor(bodySensor);
        }

        makeActorAppeared();
    }

    /**
     * @brief Updates every group.
     */
    void SupportFreezeSyncGroupHolder::movement() {
        for (s32 i = 0; i < mGroupNum; i++) {
            mGroups[i]->movement();
        }
    }

    /**
     * @brief Registers an actor to the group given by its placement link, creating the group if needed.
     * @param pActor The actor to register.
     * @param rInfo The actor init info holding the group link.
     */
    void SupportFreezeSyncGroupHolder::regist(LiveActor* pActor, const ActorInitInfo& rInfo) {
        SupportFreezeSyncGroup* group = tryFindGroup(rInfo);
        if (group == nullptr) {
            group = new SupportFreezeSyncGroup();
            group->init(rInfo);
            mGroups[mGroupNum] = group;
            mGroupNum++;
        }
        group->regist(pActor);
    }

    /**
     * @brief Finds the group matching the group link of an actor.
     * @param rInfo The actor init info holding the group link.
     * @return The matching group, or nullptr if there is none.
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

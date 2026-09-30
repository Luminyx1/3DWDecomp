#pragma once

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Scene/ISceneObj.hpp"

namespace al {
class PlacementId;

class SupportFreezeSyncGroup {
public:
    SupportFreezeSyncGroup();

    void init(const ActorInitInfo& rInfo);
    void regist(LiveActor* pActor);
    void setHostSensor(HitSensor* pSensor);
    bool isEqualGroupId(const ActorInitInfo& rInfo) const;
    void movement();

    PlacementId* mPlacementId = nullptr;
    HitSensor* mHostSensor = nullptr;
    LiveActor** mActors = nullptr;
    s32 mActorNum = 0;
    s32 mActorMax = 64;
};

class SupportFreezeSyncGroupHolder : public LiveActor, public ISceneObj {
public:
    SupportFreezeSyncGroupHolder();

    void initAfterPlacementSceneObj(const ActorInitInfo& rInfo) override;
    void movement() override;

    void regist(LiveActor* pActor, const ActorInitInfo& rInfo);
    SupportFreezeSyncGroup* tryFindGroup(const ActorInitInfo& rInfo);

    SupportFreezeSyncGroup** mGroups = nullptr;
    s32 mGroupNum = 0;
    s32 mGroupMax = 64;
};

bool registSupportFreezeSyncGroup(LiveActor* pActor, const ActorInitInfo& rInfo);
}  // namespace al

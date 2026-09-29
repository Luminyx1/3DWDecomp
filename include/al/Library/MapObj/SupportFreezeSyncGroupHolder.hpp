#pragma once

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Scene/ISceneObj.hpp"

namespace al {
class HitSensor;
class PlacementId;

class SupportFreezeSyncGroup {
public:
    SupportFreezeSyncGroup();

    void init(const ActorInitInfo&);
    void regist(LiveActor*);
    void setHostSensor(HitSensor*);
    bool isEqualGroupId(const ActorInitInfo&) const;
    void movement();

    PlacementId* mGroupId;             // _0
    HitSensor* mHostSensor = nullptr;  // _8
    LiveActor** mActors = nullptr;     // _10
    s32 mActorNum = 0;                 // _18
    s32 mActorMaxNum = 64;             // _1c
};

class SupportFreezeSyncGroupHolder : public LiveActor, public ISceneObj {
public:
    SupportFreezeSyncGroupHolder();

    void initAfterPlacementSceneObj(const ActorInitInfo&) override;
    void movement() override;

    void regist(LiveActor*, const ActorInitInfo&);
    SupportFreezeSyncGroup* tryFindGroup(const ActorInitInfo&);

    SupportFreezeSyncGroup** mGroups = nullptr;  // _150
    s32 mGroupNum = 0;                           // _158
    s32 mGroupMaxNum = 64;                       // _15c
};

bool registSupportFreezeSyncGroup(LiveActor*, const ActorInitInfo&);
}  // namespace al

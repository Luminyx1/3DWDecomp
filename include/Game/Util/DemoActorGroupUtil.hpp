#pragma once

namespace al {
    class ActorInitInfo;
    class DemoDirector;
    class LiveActor;
    struct PlacementInfo;
};  // namespace al

class DemoActorGroup;

namespace DemoActorGroupUtil {
    void initDemoActorGroup(al::LiveActor* pActor, const al::ActorInitInfo& rInfo,
                            const al::PlacementInfo& rPlacementInfo);
    void addDemoActorGroup(DemoActorGroup* pGroup, al::DemoDirector* pDemoDirector);
};  // namespace DemoActorGroupUtil

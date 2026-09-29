#pragma once

#include <math/seadVector.h>

namespace al {
    class ClippingJudge;
    class LiveActor;
    class ActorInitInfo;

    void initActorClipping(LiveActor*, const ActorInitInfo&);

    void addToHostActorClipping(LiveActor*, const LiveActor*);

    ClippingJudge* getClippingJudge(LiveActor*);

    bool isClipped(const LiveActor*);

    bool isInvalidCliping(const LiveActor*);
    void invalidateClipping(LiveActor*);
    void validateClipping(LiveActor*);
    void onDrawClipping(LiveActor*);
    void offDrawClipping(LiveActor*);
    void onUseCameraClippingPos(LiveActor*);
    void offUseCameraClippingPos(LiveActor*);

    bool tryExpandClippingByShadowLength(LiveActor*, sead::Vector3f*);
    void initGroupClipping(LiveActor*, const ActorInitInfo&, s32);
    void setClippingInfo(LiveActor*, f32, const sead::Vector3f*);
    f32 getClippingRadius(const LiveActor*);
    const sead::Vector3f& getClippingCenterPos(const LiveActor*);
};  // namespace al

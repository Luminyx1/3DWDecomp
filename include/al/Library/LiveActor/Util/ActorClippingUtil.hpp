#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
    class ActorInitInfo;
    class ClippingJudge;
    class LiveActor;

    void initActorClipping(LiveActor* pActor, const ActorInitInfo& rInfo);
    void recreateClipping(LiveActor* pActor, const ActorInitInfo& rInfo);
    void addToHostActorClipping(LiveActor* pActor, const LiveActor* pHost);
    void initGroupClipping(LiveActor* pActor, const ActorInitInfo& rInfo, s32 num);
    void resetClippingDistanceStates(LiveActor* pActor);
    void setExpandedClippingMode(LiveActor* pActor, bool isExpanded);
    bool isExpandedClippingMode(const LiveActor* pActor);
    void moveActorToClippingGroup(LiveActor* pActor, LiveActor* pGroupActor);
    void setCollisionClippingDisabled(LiveActor* pActor, bool isDisabled);
    void setClippingFarMax(LiveActor* pActor);
    f32 getClippingRadius(const LiveActor* pActor);
    void setNoCollisionClip(LiveActor* pActor, bool isNoClip);
    void setClippingInfo(LiveActor* pActor, f32 radius, const sead::Vector3f* pOffset);
    void setClippingOffset(LiveActor* pActor, const sead::Vector3f& rOffset);
    void setShadowClippingDistance(LiveActor* pActor, f32 distance);
    void setDrawClippingRadius(LiveActor* pActor, f32 radius);
    const sead::Vector3f& getClippingCenterPos(const LiveActor* pActor);
    void setClippingNearDistance(LiveActor* pActor, f32 distance);
    void setClippingNearFarDistance(LiveActor* pActor, f32 near, f32 far);
    ClippingJudge* getClippingJudge(LiveActor* pActor);
    void disableAllLOD(LiveActor* pActor);
    void enableAllLOD(LiveActor* pActor);
    void forceLodDisabled(LiveActor* pActor, bool isDisabled);
    void expandClippingRadiusByShadowLength(LiveActor* pActor, sead::Vector3f* pOffset, f32 shadowLength);
    bool tryExpandClippingToGround(LiveActor* pActor, sead::Vector3f* pOffset, f32 length);
    bool tryExpandClippingByShadowLength(LiveActor* pActor, sead::Vector3f* pOffset);
    bool tryExpandClippingByExpandObject(LiveActor* pActor, const ActorInitInfo& rInfo);
    bool isClipped(const LiveActor* pActor);
    bool isInvalidClipping(const LiveActor* pActor);
    void invalidateClipping(LiveActor* pActor);
    void validateClipping(LiveActor* pActor);
    void onDrawClipping(LiveActor* pActor);
    void offDrawClipping(LiveActor* pActor);
    void onUseCameraClippingPos(LiveActor* pActor);
    void offUseCameraClippingPos(LiveActor* pActor);

    bool isInvalidCliping(const LiveActor*);
}  // namespace al

namespace alActorFunction {
    bool isDrawClipping(const al::LiveActor* pActor);
}  // namespace alActorFunction

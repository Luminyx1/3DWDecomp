#pragma once

#include <math/seadVector.h>

namespace al {
    class ClippingJudge;
    class LiveActor;
    class ActorInitInfo;

    void initActorClipping(LiveActor* pActor, const ActorInitInfo& rInfo);
    void recreateClipping(LiveActor* pActor, const ActorInitInfo& rInfo);
    void addToHostActorClipping(LiveActor* pActor, const LiveActor* pHost);
    void initGroupClipping(LiveActor* pActor, const ActorInitInfo& rInfo, s32 groupIdx);
    void resetClippingDistanceStates(LiveActor* pActor);
    void setExpandedClippingMode(LiveActor* pActor, bool isExpanded);
    bool isExpandedClippingMode(const LiveActor* pActor);
    void moveActorToClippingGroup(LiveActor* pActor, LiveActor* pGroupActor);
    void setCollisionClippingDisabled(LiveActor* pActor, bool isDisabled);
    void setClippingFarMax(LiveActor* pActor);
    f32 getClippingRadius(const LiveActor* pActor);
    void setNoCollisionClip(LiveActor* pActor, bool isNoClip);
    void setClippingInfo(LiveActor* pActor, f32 radius, const sead::Vector3f* pCenter);
    void setClippingOffset(LiveActor* pActor, const sead::Vector3f& rOffset);
    void setShadowClippingDistance(LiveActor* pActor, f32 distance);
    void setDrawClippingRadius(LiveActor* pActor, f32 radius);
    const sead::Vector3f& getClippingCenterPos(const LiveActor* pActor);
    void setClippingNearDistance(LiveActor* pActor, f32 distance);
    void setClippingNearFarDistance(LiveActor* pActor, f32 nearDistance, f32 farDistance);
    ClippingJudge* getClippingJudge(LiveActor* pActor);
    void disableAllLOD(LiveActor* pActor);
    void enableAllLOD(LiveActor* pActor);
    void forceLodDisabled(LiveActor* pActor, bool isDisabled);
    void expandClippingRadiusByShadowLength(LiveActor* pActor, sead::Vector3f* pCenter, f32 shadowLength);
    bool tryExpandClippingToGround(LiveActor* pActor, sead::Vector3f* pCenter, f32 distance);
    bool tryExpandClippingByShadowLength(LiveActor* pActor, sead::Vector3f* pCenter);
    bool tryExpandClippingByExpandObject(LiveActor* pActor, const ActorInitInfo& rInfo);
    bool isClipped(const LiveActor* pActor);
    bool isInvalidClipping(const LiveActor* pActor);
    void invalidateClipping(LiveActor* pActor);
    void validateClipping(LiveActor* pActor);
    void onDrawClipping(LiveActor* pActor);
    void offDrawClipping(LiveActor* pActor);
    void onUseCameraClippingPos(LiveActor* pActor);
    void offUseCameraClippingPos(LiveActor* pActor);
};  // namespace al

namespace alActorFunction {
    bool isDrawClipping(const al::LiveActor* pActor);
};

#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class ActorInitInfo;
class AreaObjDirector;
class ClippingJudge;
class LiveActor;
class MultiCoreQueueThread;

class ClippingAreaActorViewHolder {
public:
    ClippingAreaActorViewHolder(s32 maxActors, MultiCoreQueueThread* pThread,
                                const AreaObjDirector* pAreaObjDirector);

    virtual void execute();
    virtual void draw() const {}
    virtual ~ClippingAreaActorViewHolder();

    void endInit();
    void recreateClipping(LiveActor* pActor, const ActorInitInfo& rInfo);
    void removeFromClippingSystem(LiveActor* pActor);
    void moveToClippingSystem(LiveActor* pActor);
    void registerActor(LiveActor* pActor, const ActorInitInfo& rInfo);
    void registerActorToHost(LiveActor* pActor, const LiveActor* pHost);
    void moveToClippingGroup(LiveActor* pActor, LiveActor* pGroupActor);
    void validateClipping(LiveActor* pActor);
    void invalidateClipping(LiveActor* pActor);
    f32 getClippingRadius(const LiveActor* pActor);
    void setClippingRadius(LiveActor* pActor, f32 radius);
    void setClippingOffset(LiveActor* pActor, const sead::Vector3f& rOffset);
    void update(ClippingJudge* pJudge);
    void waitIfPendingAsyncClipping();
    void updateNearFarClipping(LiveActor* pActor, f32 near, f32 far);
    void setShadowClippingDistance(LiveActor* pActor, f32 distance);
    void setDrawClippingRadius(LiveActor* pActor, f32 radius);
    void setNoCollisionClip(LiveActor* pActor, bool isNoClip);
    void resetClippingDistanceStates();
    void setExpandedClippingMode(bool isExpanded);

    void setCollisionClippingDisabled(bool isDisabled) { mIsCollisionClippingDisabled = isDisabled; }

    void disableForceClipAreas() { mIsForceClipAreasDisabled = true; }

    u8 _8[0x5b];
    bool mIsCollisionClippingDisabled;
    bool mIsForceClipAreasDisabled;
    u8 _65[0xe0 - 0x65];
};
}  // namespace al

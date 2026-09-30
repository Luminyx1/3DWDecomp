#pragma once

#include "Library/Clipping/ClippingDirectorBase.hpp"

namespace al {
class ClippingAreaActorViewHolder;
class MultiCoreQueueThread;

class ClippingAreaDirector : public ClippingDirectorBase {
public:
    ClippingAreaDirector(ExecuteDirector* pExecuteDirector, s32 maxActors,
                         const AreaObjDirector* pAreaObjDirector,
                         const PlayerHolder* pPlayerHolder, SceneCameraInfo* pSceneCameraInfo,
                         CameraDirector_RS* pCameraDirector, MultiCoreQueueThread* pThread);

    void execute() override;
    ~ClippingAreaDirector() override;
    void registerActor(LiveActor* pActor, const ActorInitInfo& rInfo) override;
    void recreateClipping(LiveActor* pActor, const ActorInitInfo& rInfo) override;
    void registerActorToHost(LiveActor* pActor, const LiveActor* pHost) override;
    void addToGroupClipping(LiveActor* pActor, const ActorInitInfo& rInfo, s32 num) override;
    void addToClipping(LiveActor* pActor) override;
    void removeFromClipping(LiveActor* pActor) override;
    void moveToClippingGroup(LiveActor* pActor, LiveActor* pGroupActor) override;
    void setCollisionClippingDisabled(bool isDisabled) override;
    void setActorFarClipLevel(LiveActor* pActor, s32 level) override;
    f32 getActorClippingRadius(const LiveActor* pActor) override;
    void setNoCollisionClip(LiveActor* pActor, bool isNoClip) override;
    void invalidateActorClipping(LiveActor* pActor) override;
    void validateActorClipping(LiveActor* pActor) override;
    void setActorClippingInfo(LiveActor* pActor, f32 radius,
                              const sead::Vector3f* pOffset) override;
    void setActorClippingOffset(LiveActor* pActor, const sead::Vector3f& rOffset) override;
    const sead::Vector3f& getActorClippingCenterPos(const LiveActor* pActor) override;
    void setActorNearClipDistance(LiveActor* pActor, f32 distance) override;
    void setActorNearFarClipDistance(LiveActor* pActor, f32 near, f32 far) override;
    void setShadowClippingDistance(LiveActor* pActor, f32 distance) override;
    void setDrawClippingRadius(LiveActor* pActor, f32 radius) override;
    void endInit() override;
    void resetClippingDistanceStates() override;
    void setExpandedClippingMode(bool isExpanded) override;
    void* findActorInfo(const LiveActor* pActor) const override;
    void disableForceClipAreas() override;
    void executeRequestAsyncUpdate() override;
    void waitPendingClippingRequest() override;
    void setLODDisabled(LiveActor* pActor, bool isDisabled) override;

    ClippingAreaActorViewHolder* mViewHolder;
    const PlayerHolder* mPlayerHolder;
};
}  // namespace al

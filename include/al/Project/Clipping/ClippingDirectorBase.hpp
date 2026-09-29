#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>
#include "Library/Execute/IUseExecutor.hpp"

namespace al {
    class LiveActor;
    class ActorInitInfo;
    class AreaObjDirector;
    class CameraDirector_RS;
    class ClippingActorInfo;
    class ClippingFarAreaObserver;
    class ClippingJudge;
    class ExecuteDirector;
    class PlayerHolder;
    class SceneCameraInfo;

    /// Interface of the scene's clipping director (see ClippingDirector for the implementation).
    class ClippingDirectorBase : public IUseExecutor {
    public:
        ClippingDirectorBase(ExecuteDirector* pExecuteDirector, const AreaObjDirector* pAreaObjDirector,
                             const PlayerHolder* pPlayerHolder, SceneCameraInfo* pSceneCameraInfo,
                             CameraDirector_RS* pCameraDirector);

        virtual void execute() override;

        virtual ~ClippingDirectorBase() {}

        virtual void registerActor(LiveActor* pActor, const ActorInitInfo& rInfo) = 0;
        virtual void recreateClipping(LiveActor* pActor, const ActorInitInfo& rInfo) {}
        virtual void registerActorToHost(LiveActor* pActor, const LiveActor* pHost) = 0;
        virtual void addToGroupClipping(LiveActor* pActor, const ActorInitInfo& rInfo, s32) = 0;
        virtual void addToClipping(LiveActor* pActor) = 0;
        virtual void removeFromClipping(LiveActor* pActor) = 0;
        virtual void moveToClippingGroup(LiveActor* pActor, LiveActor* pOther) {}
        virtual void setCollisionClippingDisabled(bool isDisabled) {}
        virtual void setActorFarClipLevel(LiveActor* pActor, s32 level) = 0;
        virtual f32 getActorClippingRadius(const LiveActor* pActor) = 0;
        virtual void setNoCollisionClip(LiveActor* pActor, bool isNoClip) {}
        virtual void invalidateActorClipping(LiveActor* pActor) = 0;
        virtual void validateActorClipping(LiveActor* pActor) = 0;
        virtual void setActorClippingInfo(LiveActor* pActor, f32 radius, const sead::Vector3f* pOffset) = 0;
        virtual void setActorClippingOffset(LiveActor* pActor, const sead::Vector3f& rOffset) {}
        virtual const sead::Vector3f& getActorClippingCenterPos(const LiveActor* pActor) = 0;
        virtual void setActorNearClipDistance(LiveActor* pActor, f32 distance) = 0;
        virtual void setActorNearFarClipDistance(LiveActor* pActor, f32 near, f32 far) = 0;
        virtual void setShadowClippingDistance(LiveActor* pActor, f32 distance) {}
        virtual void setDrawClippingRadius(LiveActor* pActor, f32 radius) {}
        virtual void endInit();
        virtual void resetClippingDistanceStates() {}
        virtual void setExpandedClippingMode(bool isExpanded) {}
        virtual ClippingActorInfo* findActorInfo(const LiveActor* pActor) const = 0;
        virtual void disableForceClipAreas() {}
        virtual void executeRequestAsyncUpdate() {}
        virtual void waitPendingClippingRequest() {}
        virtual void setLODDisabled(LiveActor* pActor, bool isDisabled) {}

        void setClippingJudgeUsClippingPosAsPlayerPos(bool isUse);

        static bool sLODDisabled;

        ClippingJudge* mClippingJudge;                      // _8
        ClippingFarAreaObserver* mFarAreaObserver;          // _10
        void* _18;
    };
};

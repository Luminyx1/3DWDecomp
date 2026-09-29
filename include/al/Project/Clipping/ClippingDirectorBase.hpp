#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>
#include "Library/Execute/IUseExecutor.hpp"

namespace al {
    class LiveActor;
    class ActorInitInfo;

    /// Interface of the scene's clipping director (see ClippingDirector for the implementation).
    class ClippingDirectorBase : public IUseExecutor {
    public:
        virtual ~ClippingDirectorBase();

        virtual void registerActor(LiveActor* pActor, const ActorInitInfo& rInfo) = 0;
        virtual void recreateClipping(LiveActor* pActor, const ActorInitInfo& rInfo);
        virtual void registerActorToHost(LiveActor* pActor, const LiveActor* pHost) = 0;
        virtual void addToGroupClipping(LiveActor* pActor, const ActorInitInfo& rInfo, s32) = 0;
        virtual void addToClipping(LiveActor* pActor) = 0;
        virtual void removeFromClipping(LiveActor* pActor) = 0;
        virtual void moveToClippingGroup(LiveActor* pActor, LiveActor* pOther);
        virtual void setCollisionClippingDisabled(bool isDisabled);
        virtual void setActorFarClipLevel(LiveActor* pActor, s32 level) = 0;
        virtual f32 getActorClippingRadius(const LiveActor* pActor) = 0;
        virtual void setNoCollisionClip(LiveActor* pActor, bool isNoClip);
        virtual void invalidateActorClipping(LiveActor* pActor) = 0;
        virtual void validateActorClipping(LiveActor* pActor) = 0;
        virtual void setActorClippingInfo(LiveActor* pActor, f32 radius, const sead::Vector3f* pOffset) = 0;
    };
};

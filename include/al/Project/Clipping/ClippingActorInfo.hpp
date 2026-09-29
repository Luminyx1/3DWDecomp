#pragma once

#include <math/seadVector.h>

namespace al {
    class ActorInitInfo;
    class ClippingJudge;
    class LiveActor;
    class PlacementId;

    /// Clipping state of one actor.
    class ClippingActorInfo {
    public:
        ClippingActorInfo(LiveActor* pActor);

        void setTypeToSphere(f32 radius, const sead::Vector3f* pCenter);
        void updateClipping(const ClippingJudge* pJudge);
        bool judgeClipping(const ClippingJudge* pJudge) const;
        bool isGroupClipping() const;
        void setGroupClippingId(const ActorInitInfo& rInfo);

        LiveActor* mActor;                  // _0
        const sead::Vector3f* mCenter;      // _8
        f32 mRadius;                        // _10
        f32 mNearClipDistance;              // _14
        PlacementId* mGroupClippingId;      // _18
        s16 mFarClipLevel;                  // _20
        const bool* mViewGroupFarClipFlag;  // _28
        void* _30;
    };
};

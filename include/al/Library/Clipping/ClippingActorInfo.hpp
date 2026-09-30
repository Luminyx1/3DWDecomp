#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class ActorInitInfo;
class ClippingJudge;
class LiveActor;
class PlacementId;

class ClippingActorInfo {
public:
    ClippingActorInfo(LiveActor* pActor);

    void setTypeToSphere(f32 radius, const sead::Vector3f* pPos);
    void updateClipping(const ClippingJudge* pJudge);
    bool judgeClipping(const ClippingJudge* pJudge) const;
    bool isGroupClipping() const;
    void setGroupClippingId(const ActorInitInfo& rInfo);

    LiveActor* getLiveActor() const { return mActor; }

    LiveActor* mActor;
    const sead::Vector3f* mTransPtr = nullptr;
    f32 mClippingRadius = 0.0f;
    f32 mNearClipDistance = 300.0f;
    PlacementId* mGroupClippingId;
    s16 mFarClipLevel = 1;
    const bool* mViewGroupFarClipFlag = nullptr;
    PlacementId* mPlacementId = nullptr;
};

class ClippingActorInfoList {
public:
    ClippingActorInfoList(s32 maxInfos);

    void add(ClippingActorInfo* pInfo);
    ClippingActorInfo* remove(LiveActor* pActor);
    ClippingActorInfo* find(const LiveActor* pActor, s32* pIndex) const;
    ClippingActorInfo* tryFind(const LiveActor* pActor) const;
    bool isInList(const LiveActor* pActor) const;

    s32 mMaxInfos = 0;
    s32 mNumInfos = 0;
    ClippingActorInfo** mInfos;
};

class ClippingActorHolder {
public:
    ClippingActorHolder(s32 maxActors);

    void update(const ClippingJudge* pJudge);
    ClippingActorInfo* registerActor(LiveActor* pActor);
    ClippingActorInfo* initGroupClipping(LiveActor* pActor, const ActorInitInfo& rInfo);
    void validateClipping(LiveActor* pActor);
    void invalidateClipping(LiveActor* pActor);
    void addToClippingTarget(LiveActor* pActor);
    void removeFromClippingTarget(LiveActor* pActor);
    f32 getClippingRadius(const LiveActor* pActor);
    ClippingActorInfo* find(const LiveActor* pActor) const;
    const sead::Vector3f& getClippingCenterPos(const LiveActor* pActor);
    void setTypeToSphere(LiveActor* pActor, f32 radius, const sead::Vector3f* pPos);
    void setNearClipDistance(LiveActor* pActor, f32 distance);
    void setFarClipLevel(LiveActor* pActor, s32 level);

    s32 mMaxActors = 0;
    s32 mNumActors = 0;
    ClippingActorInfoList* mClippingTargets = nullptr;
    ClippingActorInfoList* mInvalidClippings = nullptr;
    ClippingActorInfoList* mNonClippingTargets = nullptr;
    ClippingActorInfoList* mGroupClippings = nullptr;
};
}  // namespace al

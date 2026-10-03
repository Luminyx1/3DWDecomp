#pragma once

#include <basis/seadTypes.h>
#include <container/seadObjArray.h>
#include <container/seadPtrArray.h>
#include <math/seadVector.h>

#include "Library/Execute/IUseExecutor.hpp"
#include "Project/Clipping/ClippingAreaActorInfo.hpp"
#include "Project/Clipping/ClippingAreaActorInfoList.hpp"
#include "Project/Framework/MultiCoreQueueThread.hpp"

namespace al {
class ActorInitInfo;
class AreaObj;
class AreaObjDirector;
class AreaObjGroup;
class ClippingJudge;
class LiveActor;

/**
 * Holds the clipping nodes of all actors and updates the area based clipping, possibly on a
 * worker thread.
 */
class ClippingAreaActorViewHolder : public IUseExecutor, public MultiCoreQueueExecutor {
public:
    /**
     * Pending request of a node registered while the clipping is being updated.
     */
    enum RequestType {
        RequestType_None = 0,
        RequestType_Remove = 1,
        RequestType_Move = 2,
    };

    ClippingAreaActorViewHolder(s32 maxActors, MultiCoreQueueThread* pThread,
                                const AreaObjDirector* pAreaObjDirector);

    void execute() override;
    virtual ~ClippingAreaActorViewHolder();

    const char* executorName() const override { return "ClippingAreaDirector"; }

    void executeOnThread() override;

    void endInit();
    ClippingAreaActorInfoNode* createAndAssignActorInfo(LiveActor* pActor);
    void recreateClipping(LiveActor* pActor, const ActorInitInfo& rInfo);
    void removeFromClippingSystem(LiveActor* pActor);
    void finishInitActorClipping(LiveActor* pActor, const ActorInitInfo& rInfo,
                                 ClippingAreaActorInfoNode* pNode);
    void moveToClippingSystem(LiveActor* pActor);
    void registerActor(LiveActor* pActor, const ActorInitInfo& rInfo);
    void registerActorToHost(LiveActor* pActor, const LiveActor* pHost);
    void moveToClippingGroup(LiveActor* pActor, LiveActor* pGroupActor);
    void validateClipping(LiveActor* pActor);
    void invalidateClipping(LiveActor* pActor);
    f32 getClippingRadius(const LiveActor* pActor);
    void setClippingRadius(LiveActor* pActor, f32 radius);
    void setClippingOffset(LiveActor* pActor, const sead::Vector3f& rOffset);
    void dequeueNodeList();
    void internalUpdate();
    void updateRequestLists();
    void update(ClippingJudge* pJudge);
    void waitIfPendingAsyncClipping();
    bool isInForceClipViewCtrlArea(const sead::Vector3f& rPos);
    void updateNearFarClipping(LiveActor* pActor, f32 near, f32 far);
    void setShadowClippingDistance(LiveActor* pActor, f32 distance);
    void setDrawClippingRadius(LiveActor* pActor, f32 radius);
    void setNoCollisionClip(LiveActor* pActor, bool isNoClip);
    void resetClippingDistanceStates();
    void setExpandedClippingMode(bool isExpanded);

    void setCollisionClippingDisabled(bool isDisabled) {
        mIsCollisionClippingDisabled = isDisabled;
    }

    void disableForceClipAreas() { mIsForceClipAreasDisabled = true; }

    ClippingJudge* getJudge() const { return mJudge; }

    s32 mMaxNodeNum;
    s32 mNodeNum = 0;
    ClippingAreaActorInfoNode* mNodes;
    ClippingAreaActorInfoList* mUnclippedInfoList;
    ClippingAreaActorInfoList* mClippedInfoList;
    ClippingAreaActorInfoList* mInvalidInfoList;
    ClippingAreaActorInfoNodeList* mInvalidNodeList;
    sead::ObjArray<ClippingAreaActorInfo> mInfos;
    bool mIsUpdating = false;
    bool mIsAsyncPending = false;
    bool mIsFirstUpdate = true;
    bool mIsCollisionClippingDisabled = false;
    bool mIsForceClipAreasDisabled = false;
    sead::PtrArray<ClippingAreaActorInfoNode> mRequestNodes;
    sead::PtrArray<LiveActor> mStartClippedRequests;
    sead::PtrArray<LiveActor> mEndClippedRequests;
    sead::PtrArray<LiveActor> mStartFarLodRequests;
    sead::PtrArray<LiveActor> mEndFarLodRequests;
    MultiCoreQueueThread* mThread;
    ClippingJudge* mJudge = nullptr;
    const AreaObjDirector* mAreaObjDirector;
    AreaObjGroup* mForceClipAreaGroup = nullptr;
    AreaObj* mForceClipArea = nullptr;
};

static_assert(sizeof(ClippingAreaActorViewHolder) == 0xe0);
}  // namespace al

#include "Project/Clipping/ClippingAreaActorViewHolder.hpp"

#include "Library/Clipping/ClippingJudge.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/LiveActorFlag.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementId.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Project/AreaObj/AreaObj.hpp"
#include "Project/AreaObj/AreaObjDirector.hpp"
#include "Project/AreaObj/AreaObjGroup.hpp"

namespace al {
/**
 * Creates the view holder.
 * @param maxActors maximum number of actors handled
 * @param pThread thread used for the asynchronous update
 * @param pAreaObjDirector area director
 */
ClippingAreaActorViewHolder::ClippingAreaActorViewHolder(s32 maxActors,
                                                         MultiCoreQueueThread* pThread,
                                                         const AreaObjDirector* pAreaObjDirector)
    : mMaxNodeNum(maxActors), mAreaObjDirector(pAreaObjDirector) {
    mNodes = new ClippingAreaActorInfoNode[maxActors];
    mUnclippedInfoList = new ClippingAreaActorInfoList(this, mMaxNodeNum);
    mClippedInfoList = new ClippingAreaActorInfoList(this, mMaxNodeNum);
    mInvalidInfoList = new ClippingAreaActorInfoList(this, mMaxNodeNum);
    mInvalidNodeList = new ClippingAreaActorInfoNodeList();
    mRequestNodes.allocBuffer(mMaxNodeNum, nullptr);
    mInfos.allocBuffer(mMaxNodeNum, nullptr);
    mStartClippedRequests.allocBuffer(mMaxNodeNum, nullptr);
    mEndClippedRequests.allocBuffer(mMaxNodeNum, nullptr);
    mStartFarLodRequests.allocBuffer(mMaxNodeNum, nullptr);
    mEndFarLodRequests.allocBuffer(mMaxNodeNum, nullptr);
    mThread = pThread;
}

/**
 * Finishes initialization by looking up the force clip areas.
 */
void ClippingAreaActorViewHolder::endInit() {
    if (mAreaObjDirector != nullptr) {
        mForceClipAreaGroup = mAreaObjDirector->getAreaObjGroup("ForceClipViewCtrlArea");
    }
}

/**
 * Waits for the asynchronous update before destroying the holder.
 */
ClippingAreaActorViewHolder::~ClippingAreaActorViewHolder() {
    mThread->waitDone();
}

/**
 * Assigns a free clipping node to an actor.
 * @param pActor actor
 * @return the assigned node
 */
ClippingAreaActorInfoNode*
ClippingAreaActorViewHolder::createAndAssignActorInfo(LiveActor* pActor) {
    LiveActorFlag* flags = pActor->getFlags();
    flags->isInvalidClipping = false;
    flags->_1c = false;

    ClippingAreaActorInfoNode* node = &mNodes[mNodeNum++];
    node->mActor = pActor;
    pActor->mClippingInfoNode = node;
    return node;
}

/**
 * Recreates the clipping info of an actor that isn't part of a placed view group.
 * @param pActor actor
 * @param rInfo actor init info
 */
void ClippingAreaActorViewHolder::recreateClipping(LiveActor* pActor, const ActorInitInfo& rInfo) {
    ClippingAreaActorInfoNode* node = pActor->getClippingInfoNode();
    if (node == nullptr) {
        return;
    }

    ClippingAreaActorInfo* info = node->mInfo;
    if (info == nullptr || info->mPlacementId != nullptr) {
        return;
    }

    s32 isActorClipped = true;  // an int rather than a bool in the original code
    if (!isClipped(pActor)) {
        removeFromClippingSystem(pActor);
        isActorClipped = false;
    }

    info->removeActor(node);
    mInvalidInfoList->removeInfo(info);
    finishInitActorClipping(pActor, rInfo, node);
    if (node->mInfo != nullptr) {
        node->mInfo->disableFarLod();
    }

    if (!isActorClipped) {
        moveToClippingSystem(pActor);
    }
}

/**
 * Removes an actor from the clipping system.
 * @param pActor actor
 */
void ClippingAreaActorViewHolder::removeFromClippingSystem(LiveActor* pActor) {
    ClippingAreaActorInfoNode* node = pActor->getClippingInfoNode();
    if (node == nullptr) {
        return;
    }

    if (mIsUpdating) {
        if (node->mRequestType != RequestType_None) {
            return;
        }

        mRequestNodes.pushBack(node);
        node->mRequestType = RequestType_Remove;
        return;
    }

    if (!node->mIsDrawVisible) {
        node->mIsDrawVisible = true;
        setDisableDraw(pActor, false);
    }

    if (!node->mIsShadowVisible) {
        node->mIsShadowVisible = true;
        setDisableDepthShadow(pActor, false, false);
    }

    ClippingAreaActorInfo* info = node->mInfo;
    if (node->mIsInvalidClipping) {
        if (!info->mNodeList.isEmpty()) {
            return;
        }
    } else {
        pActor->mGlobalAlphaLastFrame = 1.0f;
        bool isEmpty = info->removeActor(node);
        mInvalidNodeList->pushBack(node);
        if (!isEmpty) {
            return;
        }
    }

    if (info->mClippingState == 2) {
        return;
    }

    if (info->mClippingState == 1) {
        mClippedInfoList->removeInfo(info);
    } else {
        mUnclippedInfoList->removeInfo(info);
    }

    mInvalidInfoList->registerInfo(info);
    info->mClippingState = 2;
}

/**
 * Assigns the clipping info of an actor from its placement.
 * @param pActor actor
 * @param rInfo actor init info
 * @param pNode clipping node of the actor
 */
void ClippingAreaActorViewHolder::finishInitActorClipping(LiveActor* pActor,
                                                          const ActorInitInfo& rInfo,
                                                          ClippingAreaActorInfoNode* pNode) {
    PlacementInfo placementInfo;
    pNode->mIsUseOwnClipping = tryGetLinksInfo(&placementInfo, rInfo, "ViewGroupSingleClipping");

    ClippingAreaActorInfo* info;
    if (pNode->mIsUseOwnClipping || tryGetLinksInfo(&placementInfo, rInfo, "ViewGroup") ||
        tryGetLinksInfo(&placementInfo, rInfo, "GroupClipping")) {
        bool isUseOnlyDistance = false;
        tryGetArg(&isUseOnlyDistance, placementInfo, "UseOnlyDistance");
        bool isUseDrawClipping = false;
        tryGetArg(&isUseDrawClipping, placementInfo, "UseDrawClipping");

        if (isUseOnlyDistance) {
            info = mInfos.emplaceBack(pNode, rInfo, &placementInfo);
            mInvalidInfoList->registerInfo(info);
        } else {
            PlacementId placementId;
            placementId.init(placementInfo);

            bool isFound = false;
            for (auto it = mInfos.begin(); it != mInfos.end(); ++it) {
                info = &*it;
                if (info->isViewInfo(placementId)) {
                    isFound = true;
                    break;
                }
            }

            if (!isFound) {
                info = mInfos.emplaceBack(placementInfo, rInfo);
                mInvalidInfoList->registerInfo(info);
            }

            info->registerActor(pNode, false);
        }

        if (isUseDrawClipping) {
            onDrawClipping(pActor);
        }
    } else {
        info = mInfos.emplaceBack(pNode, rInfo, nullptr);
        mInvalidInfoList->registerInfo(info);
    }

    info->removeActor(pNode);
    mInvalidNodeList->pushBack(pNode);
}

/**
 * Adds an actor to the clipping system.
 * @param pActor actor
 */
void ClippingAreaActorViewHolder::moveToClippingSystem(LiveActor* pActor) {
    if (isInvalidClipping(pActor)) {
        return;
    }

    ClippingAreaActorInfoNode* node = pActor->getClippingInfoNode();
    if (node == nullptr) {
        return;
    }

    if (mIsUpdating) {
        if (node->mRequestType != RequestType_None) {
            return;
        }

        mRequestNodes.pushBack(node);
        node->mRequestType = RequestType_Move;
        return;
    }

    ClippingAreaActorInfo* info = node->mInfo;
    bool isRegistered = false;
    if (node->mIsInvalidClipping) {
        if (isClipped(pActor)) {
            pActor->mGlobalAlphaLastFrame = 0.0f;
        }

        mInvalidNodeList->erase(node);
        info->registerActor(node, false);
        isRegistered = true;
    }

    if (info->mClippingState == 2) {
        mInvalidInfoList->removeInfo(info);
        info->mClippingState = 0;
        mUnclippedInfoList->registerInfo(info);
        if (isClipped(pActor)) {
            pActor->endClipped();
        }

        info->mIsSwitchChanged = true;
        return;
    }

    if (!isRegistered) {
        return;
    }

    if (info->mClippingState == 1) {
        if (!isClipped(pActor)) {
            pActor->startClipped();
        }
    } else if (info->mClippingState == 0) {
        if (isClipped(pActor)) {
            pActor->endClipped();
        }
    }
}

/**
 * Registers an actor to the clipping system.
 * @param pActor actor
 * @param rInfo actor init info
 */
void ClippingAreaActorViewHolder::registerActor(LiveActor* pActor, const ActorInitInfo& rInfo) {
    if (pActor->mIsFarLodModel || pActor->getClippingInfoNode() != nullptr) {
        return;
    }

    finishInitActorClipping(pActor, rInfo, createAndAssignActorInfo(pActor));
}

/**
 * Registers an actor to the clipping group of its host.
 * @param pActor actor
 * @param pHost host actor
 */
void ClippingAreaActorViewHolder::registerActorToHost(LiveActor* pActor, const LiveActor* pHost) {
    if (pActor->mIsFarLodModel || pActor->getClippingInfoNode() != nullptr) {
        return;
    }

    ClippingAreaActorInfoNode* hostNode = pHost->getClippingInfoNode();
    if (hostNode->mInfo->mPlacementId == nullptr) {
        return;
    }

    ClippingAreaActorInfoNode* node = createAndAssignActorInfo(pActor);
    node->mIsUseOwnClipping = hostNode->mIsUseOwnClipping;
    ClippingAreaActorInfo* info = hostNode->mInfo;
    info->registerActor(node, false);
    info->removeActor(node);
    mInvalidNodeList->pushBack(node);
}

/**
 * Moves a dead actor into the clipping group of another actor.
 * @param pActor actor owning the group
 * @param pGroupActor actor to move
 */
void ClippingAreaActorViewHolder::moveToClippingGroup(LiveActor* pActor, LiveActor* pGroupActor) {
    ClippingAreaActorInfoNode* node = pActor->getClippingInfoNode();
    ClippingAreaActorInfoNode* groupNode = pGroupActor->getClippingInfoNode();
    if (node == nullptr || groupNode == nullptr || !isDead(pGroupActor)) {
        return;
    }

    ClippingAreaActorInfo* info = node->mInfo;
    if (info == groupNode->mInfo) {
        return;
    }

    if (groupNode->mInfo != nullptr) {
        removeFromClippingSystem(pGroupActor);
    }

    info->registerActor(groupNode, true);
}

/**
 * Validates the clipping of an actor.
 * @param pActor actor
 */
void ClippingAreaActorViewHolder::validateClipping(LiveActor* pActor) {
    if (pActor->getClippingInfoNode() == nullptr) {
        return;
    }

    LiveActorFlag* flags = pActor->getFlags();
    flags->isInvalidClipping = false;
    flags->_1c = false;
    if (isDead(pActor)) {
        return;
    }

    moveToClippingSystem(pActor);
}

/**
 * Invalidates the clipping of an actor.
 * @param pActor actor
 */
void ClippingAreaActorViewHolder::invalidateClipping(LiveActor* pActor) {
    LiveActorFlag* flags = pActor->getFlags();
    flags->isInvalidClipping = true;
    flags->_1c = false;
    pActor->endFarLod();
    removeFromClippingSystem(pActor);
    if (isClipped(pActor)) {
        pActor->endClipped();
    }
}

/**
 * Gets the clipping radius of an actor.
 * @param pActor actor
 * @return the clipping radius, or 0 if the actor isn't registered
 */
f32 ClippingAreaActorViewHolder::getClippingRadius(const LiveActor* pActor) {
    ClippingAreaActorInfoNode* node = pActor->getClippingInfoNode();
    if (node == nullptr) {
        return 0.0f;
    }

    return node->mClippingRadius;
}

/**
 * Sets the clipping radius of an actor.
 * @param pActor actor
 * @param radius clipping radius
 */
void ClippingAreaActorViewHolder::setClippingRadius(LiveActor* pActor, f32 radius) {
    ClippingAreaActorInfoNode* node = pActor->getClippingInfoNode();
    if (node == nullptr) {
        return;
    }

    ClippingAreaActorInfo* info = node->mInfo;
    node->mClippingRadius = radius;
    if (info->mPlacementId == nullptr) {
        info->setClippingRadius(radius);
    }
}

/**
 * Sets the clipping offset of an actor.
 * @param pActor actor
 * @param rOffset clipping offset
 */
void ClippingAreaActorViewHolder::setClippingOffset(LiveActor* pActor,
                                                    const sead::Vector3f& rOffset) {
    ClippingAreaActorInfoNode* node = pActor->getClippingInfoNode();
    if (node == nullptr) {
        return;
    }

    node->mClippingOffset = rOffset;
}

/**
 * Handles the requests made while the clipping was being updated.
 */
void ClippingAreaActorViewHolder::dequeueNodeList() {
    for (s32 i = 0; i < mRequestNodes.size(); i++) {
        ClippingAreaActorInfoNode* node = mRequestNodes.unsafeAt(i);
        if (node->mRequestType == RequestType_Move) {
            node->mRequestType = RequestType_None;
            moveToClippingSystem(node->mActor);
        } else if (node->mRequestType == RequestType_Remove) {
            node->mRequestType = RequestType_None;
            removeFromClippingSystem(node->mActor);
        }
    }

    mRequestNodes.clear();
}

/**
 * Updates the clipping of all infos.
 */
void ClippingAreaActorViewHolder::internalUpdate() {
    mIsUpdating = true;
    if (mForceClipAreaGroup != nullptr) {
        mForceClipArea = mForceClipAreaGroup->getInFirstAreaObj(mJudge->mCameraPos);
    }

    mUnclippedInfoList->updateUnclipped(mClippedInfoList->getQueue());
    mClippedInfoList->updateClipped(mUnclippedInfoList->getQueue());
    mIsUpdating = false;
    mClippedInfoList->dequeueList();
    mUnclippedInfoList->dequeueList();
    dequeueNodeList();
}

/**
 * Applies the clipping and far LOD state changes requested by the last update.
 */
void ClippingAreaActorViewHolder::updateRequestLists() {
    mIsUpdating = true;
    for (auto it = mStartFarLodRequests.begin(); it != mStartFarLodRequests.end(); ++it) {
        it->startFarLod();
    }

    mStartFarLodRequests.clear();
    for (auto it = mEndFarLodRequests.begin(); it != mEndFarLodRequests.end(); ++it) {
        it->endFarLod();
    }

    mEndFarLodRequests.clear();
    for (auto it = mStartClippedRequests.begin(); it != mStartClippedRequests.end(); ++it) {
        LiveActor* actor = &*it;
        if (!isClipped(actor) && !isDead(actor)) {
            actor->startClipped();
        }
    }

    mStartClippedRequests.clear();
    for (auto it = mEndClippedRequests.begin(); it != mEndClippedRequests.end(); ++it) {
        LiveActor* actor = &*it;
        if (isClipped(actor)) {
            actor->endClipped();
        }
    }

    mEndClippedRequests.clear();
    mIsUpdating = false;
}

/**
 * Updates the clipping.
 * @param pJudge clipping judge
 */
void ClippingAreaActorViewHolder::update(ClippingJudge* pJudge) {
    mJudge = pJudge;
    if (mIsFirstUpdate) {
        internalUpdate();
        mIsFirstUpdate = false;
    } else {
        waitIfPendingAsyncClipping();
    }

    updateRequestLists();
    dequeueNodeList();
}

/**
 * Waits for the asynchronous update if one is running.
 */
void ClippingAreaActorViewHolder::waitIfPendingAsyncClipping() {
    if (mIsAsyncPending) {
        mThread->waitDone();
        mIsAsyncPending = false;
    }
}

/**
 * Checks whether a position is inside the current force clip area.
 * @param rPos position
 * @return whether the position is inside the area
 */
bool ClippingAreaActorViewHolder::isInForceClipViewCtrlArea(const sead::Vector3f& rPos) {
    return mForceClipArea != nullptr && mForceClipArea->isInVolume(rPos);
}

/**
 * Starts the asynchronous update.
 */
void ClippingAreaActorViewHolder::execute() {
    mIsAsyncPending = true;
    mThread->requestExecute(this);
}

/**
 * Runs the update on the worker thread.
 */
void ClippingAreaActorViewHolder::executeOnThread() {
    internalUpdate();
}

/**
 * Updates the near and far clipping distances of an actor.
 * @param pActor actor
 * @param near near distance
 * @param far far distance
 */
void ClippingAreaActorViewHolder::updateNearFarClipping(LiveActor* pActor, f32 near, f32 far) {
    ClippingAreaActorInfoNode* node = pActor->getClippingInfoNode();
    if (node == nullptr) {
        return;
    }

    ClippingAreaActorInfo* info = node->mInfo;
    if (info->mPlacementId == nullptr) {
        info->updateNarFarClip(near, far);
    }
}

/**
 * Sets the shadow clipping distance of an actor.
 * @param pActor actor
 * @param distance shadow clipping distance
 */
void ClippingAreaActorViewHolder::setShadowClippingDistance(LiveActor* pActor, f32 distance) {
    ClippingAreaActorInfoNode* node = pActor->getClippingInfoNode();
    if (node == nullptr) {
        return;
    }

    node->mInfo->mIsShadowClipping = true;
    node->mShadowClippingDistance = distance * distance;
}

/**
 * Sets the draw clipping radius of an actor.
 * @param pActor actor
 * @param radius draw clipping radius
 */
void ClippingAreaActorViewHolder::setDrawClippingRadius(LiveActor* pActor, f32 radius) {
    ClippingAreaActorInfoNode* node = pActor->getClippingInfoNode();
    if (node == nullptr) {
        return;
    }

    node->mDrawClippingRadius = radius;
}

/**
 * Sets whether the collision of an actor is kept when clipped.
 * @param pActor actor
 * @param isNoClip whether the collision isn't clipped
 */
void ClippingAreaActorViewHolder::setNoCollisionClip(LiveActor* pActor, bool isNoClip) {
    ClippingAreaActorInfoNode* node = pActor->getClippingInfoNode();
    if (node == nullptr) {
        return;
    }

    node->mIsNoCollisionClip = isNoClip;
}

/**
 * Resets the clipping distance states of all infos.
 */
void ClippingAreaActorViewHolder::resetClippingDistanceStates() {
    for (auto it = mInfos.begin(); it != mInfos.end(); ++it) {
        it->mIsSwitchChanged = true;
    }
}

/**
 * Sets whether the clipping uses expanded distances.
 * @param isExpanded whether to expand the clipping distances
 */
void ClippingAreaActorViewHolder::setExpandedClippingMode(bool isExpanded) {
    for (auto it = mInfos.begin(); it != mInfos.end(); ++it) {
        it->setExpandedClippingMode(isExpanded);
    }
}
}  // namespace al

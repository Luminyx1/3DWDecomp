#include "Project/Clipping/ClippingAreaActorInfo.hpp"

#include <attributes.h>

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/Clipping/ClippingDirectorBase.hpp"
#include "Library/Clipping/ClippingJudge.hpp"
#include "Library/Clipping/ClippingViewFadeInAreas.hpp"
#include "Library/LiveActor/ActorCollisionFunction.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementId.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Library/StageSwitch/Core/StageSwitchKeeper.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Library/Thread/Functor.hpp"
#include "Project/AreaObj/AreaObjGroup.hpp"
#include "Project/Clipping/ClippingAreaActorViewArea.hpp"
#include "Project/Clipping/ClippingAreaActorViewHolder.hpp"

namespace {
struct ClippingPreset {
    f32 mNearDistance;
    f32 mFarDistance;
};

const ClippingPreset sClippingPresets[] = {
    {-1.0f, -1.0f},       {8000.0f, 8500.0f}, {15000.0f, 17000.0f}, {20000.0f, 20000.0f},
    {10000.0f, 10500.0f}, {20000.0f, 20000.0f}, {8000.0f, 8500.0f},  {8000.0f, 8500.0f},
    {8000.0f, 8500.0f},   {7000.0f, 7500.0f},   {8000.0f, 8500.0f},
};
}  // namespace

namespace al {
NOINLINE static void updateDrawClipping(ClippingAreaActorInfoNode* pNode, ClippingJudge* pJudge);

/**
 * Applies an alpha to every actor of the group and updates the per-actor clipping.
 * @param alpha alpha to apply
 * @param pHolder holder receiving clipping requests (may be null)
 */
void ClippingAreaActorInfo::updateAlphaOnList(f32 alpha, ClippingAreaActorViewHolder* pHolder) {
    if (pHolder != nullptr && mIsUseNodeClipping) {
        ClippingJudge* judge = pHolder->mJudge;

        for (ClippingAreaActorInfoNode& node : mNodeList.robustRange()) {
            LiveActor* actor = node.mActor;
            actor->mGlobalAlpha = alpha;

            if (node.mIsUseOwnClipping && node.mClippingRadius > 0.0f) {
                sead::Vector3f pos = getTrans(actor) + node.mClippingOffset;

                if (judge->isJudgedToClipFrustum(pos, node.mClippingRadius, 200.0f, 1)) {
                    if (!isDead(actor)) {
                        pHolder->mStartClippedRequests.pushBack(actor);
                        continue;
                    }
                } else {
                    pHolder->mEndClippedRequests.pushBack(actor);
                }
            }

            if (alpha > 0.0f) {
                updateDrawClipping(&node, judge);
            }
        }
    } else {
        ClippingJudge* judge = pHolder != nullptr ? pHolder->mJudge : nullptr;

        for (ClippingAreaActorInfoNode& node : mNodeList.robustRange()) {
            node.mActor->mGlobalAlpha = alpha;

            if (alpha > 0.0f && judge != nullptr) {
                updateDrawClipping(&node, judge);
            }
        }
    }
}

/**
 * Hides or shows an actor depending on whether its draw bounds are in the frustum.
 * @param pNode node of the actor
 * @param pJudge clipping judge
 */
NOINLINE static void updateDrawClipping(ClippingAreaActorInfoNode* pNode, ClippingJudge* pJudge) {
    sead::Vector3f pos;

    if (pNode->mDrawClippingRadius > 0.0f) {
        LiveActor* actor = pNode->mActor;
        pos = getTrans(actor) + pNode->mClippingOffset;

        if (pJudge->isJudgedToClipFrustum(pos, pNode->mDrawClippingRadius, 200.0f, 1)) {
            if (pNode->mIsDrawVisible) {
                pNode->mIsDrawVisible = false;
                setDisableDraw(actor, true);
            }
        } else if (!pNode->mIsDrawVisible) {
            pNode->mIsDrawVisible = true;
            setDisableDraw(actor, false);
        }
    }
}

/**
 * Enables or disables the collision of the group depending on the distance.
 * @param rPos observer position
 * @param pCenter center of the group
 * @param radius radius of the group
 * @param isUnused unused
 */
void ClippingAreaActorInfo::updateOnlyCollisionSettings(const sead::Vector3f& rPos,
                                                        const sead::Vector3f* pCenter, f32 radius,
                                                        bool isUnused) {
    // only used by debug code that is compiled out in release builds
    if (isUnused) {
    }

    f32 distance = (rPos - *pCenter).squaredLength() - radius * radius;

    if (distance > 18000.0f * 18000.0f) {
        if (mCollisionState == 0) {
            mCollisionState = 1;
            f32 alpha = mAlpha;

            for (ClippingAreaActorInfoNode& node : mNodeList.robustRange()) {
                LiveActor* actor = node.mActor;

                if (node.mIsCollisionEnabled) {
                    if (!node.mIsNoCollisionClip) {
                        disableAllCollisionParts(actor);
                    }

                    node.mIsCollisionEnabled = false;
                }

                if (node.mShadowClippingDistance <= 0.0f) {
                    setDisableDepthShadow(actor, true, false);
                }

                actor->mGlobalAlpha = alpha;
            }
        }
    } else if (mCollisionState == 1) {
        mCollisionState = 0;
        f32 alpha = mAlpha;

        for (ClippingAreaActorInfoNode& node : mNodeList.robustRange()) {
            LiveActor* actor = node.mActor;

            if (!node.mIsCollisionEnabled) {
                enableAllCollisionParts(actor);
                node.mIsCollisionEnabled = true;
            }

            if (node.mShadowClippingDistance <= 0.0f) {
                setDisableDepthShadow(actor, false, false);
            }

            actor->mGlobalAlpha = alpha;
        }
    }
}

/**
 * Applies an alpha and enables or disables the collision of the group depending on the distance.
 * @param alpha alpha to apply
 * @param rPos observer position
 * @param pCenter center of the group
 * @param radius radius of the group
 * @param isUnused unused
 */
void ClippingAreaActorInfo::updateCollisionSettings(f32 alpha, const sead::Vector3f& rPos,
                                                    const sead::Vector3f* pCenter, f32 radius,
                                                    bool isUnused) {
    // only used by debug code that is compiled out in release builds
    if (isUnused) {
    }

    f32 distance = (rPos - *pCenter).squaredLength() - radius * radius;

    if (ClippingDirectorBase::sCollisionForcedOn) {
        distance = 0.0f;
    }

    if (distance > 18000.0f * 18000.0f) {
        if (mCollisionState == 0) {
            mCollisionState = 1;

            for (ClippingAreaActorInfoNode& node : mNodeList.robustRange()) {
                LiveActor* actor = node.mActor;

                if (node.mIsCollisionEnabled) {
                    if (!node.mIsNoCollisionClip) {
                        disableAllCollisionParts(actor);
                    }

                    node.mIsCollisionEnabled = false;
                }

                if (node.mShadowClippingDistance <= 0.0f) {
                    setDisableDepthShadow(actor, true, false);
                }

                actor->mGlobalAlpha = alpha;
            }
        } else {
            for (ClippingAreaActorInfoNode& node : mNodeList.robustRange()) {
                node.mActor->mGlobalAlpha = alpha;
            }
        }
    } else if (mCollisionState == 1) {
        mCollisionState = 0;

        for (ClippingAreaActorInfoNode& node : mNodeList.robustRange()) {
            LiveActor* actor = node.mActor;

            if (!node.mIsCollisionEnabled) {
                enableAllCollisionParts(actor);
                node.mIsCollisionEnabled = true;
            }

            if (node.mShadowClippingDistance <= 0.0f) {
                setDisableDepthShadow(actor, false, false);
            }

            actor->mGlobalAlpha = alpha;
        }
    } else {
        for (ClippingAreaActorInfoNode& node : mNodeList.robustRange()) {
            node.mActor->mGlobalAlpha = alpha;
        }
    }
}

/**
 * Shows or hides the depth shadow of every actor depending on the distance to the observer.
 * @param rPos observer position
 */
void ClippingAreaActorInfo::updateShadowClipping(const sead::Vector3f& rPos) {
    if (!mIsShadowClipping) {
        return;
    }

    for (ClippingAreaActorInfoNode& node : mNodeList.robustRange()) {
        if (node.mShadowClippingDistance > 0.0f) {
            LiveActor* actor = node.mActor;
            f32 distance = (rPos - getTrans(actor)).squaredLength();

            if (distance >= node.mShadowClippingDistance) {
                if (node.mIsShadowVisible) {
                    node.mIsShadowVisible = false;
                    setDisableDepthShadow(actor, true, false);
                }
            } else if (!node.mIsShadowVisible) {
                node.mIsShadowVisible = true;
                setDisableDepthShadow(actor, false, false);
            }
        }
    }
}

/**
 * Reads the near and far clipping distances from the placement.
 * @param rPlacementInfo placement
 */
void ClippingAreaActorInfo::readClippingDistance(const PlacementInfo& rPlacementInfo) {
    tryGetArg(&mNearDistance, rPlacementInfo, "NearDistance");

    if (mNearDistance < 0.0f) {
        mNearDistance = 7500.0f;
    }

    tryGetArg(&mFarDistance, rPlacementInfo, "FarDistance");

    if (mFarDistance < 0.0f) {
        mFarDistance = 8000.0f;
    }

    s32 preset = 0;
    bool isExistPreset = tryGetArg(&preset, rPlacementInfo, "ClippingPreset");

    if (preset <= 10 && isExistPreset && preset != 0) {
        mNearDistance = sClippingPresets[preset].mNearDistance;
        mFarDistance = sClippingPresets[preset].mFarDistance;
    }
}

/**
 * Creates the view area linked to the placement.
 * @param rPlacementInfo placement
 * @param rInfo actor init info
 * @return whether a view area was created
 */
bool ClippingAreaActorInfo::initViewArea(const PlacementInfo& rPlacementInfo,
                                         const ActorInitInfo& rInfo) {
    PlacementInfo linkInfo;

    if (tryGetLinksInfo(&linkInfo, rPlacementInfo, "ViewArea")) {
        mViewArea = new ClippingAreaActorViewArea(linkInfo, rInfo);
        return true;
    }

    return false;
}

/**
 * Initializes the stage switches toggling the full visibility of the group.
 * @param rPlacementInfo placement
 * @param rInfo actor init info
 */
void ClippingAreaActorInfo::initStageSwitchKeeper(const PlacementInfo& rPlacementInfo,
                                                  const ActorInitInfo& rInfo) {
    if (!tryInitStageSwitch(this, rInfo.getStageSwitchDirector(), rPlacementInfo)) {
        return;
    }

    listenStageSwitchOn(this, "SwitchFullVisibility",
                        Functor(this, &ClippingAreaActorInfo::switchMaxDistanceActivate));
    listenStageSwitchOff(this, "SwitchFullVisibility",
                         Functor(this, &ClippingAreaActorInfo::switchMaxDistanceDeactivate));
    listenStageSwitchOn(this, "SwitchFullVisibility1",
                        Functor(this, &ClippingAreaActorInfo::switchMaxDistanceActivate));
    listenStageSwitchOff(this, "SwitchFullVisibility1",
                         Functor(this, &ClippingAreaActorInfo::switchMaxDistanceDeactivate));
    listenStageSwitchOn(this, "SwitchFullVisibility2",
                        Functor(this, &ClippingAreaActorInfo::switchMaxDistanceActivate));
    listenStageSwitchOff(this, "SwitchFullVisibility2",
                         Functor(this, &ClippingAreaActorInfo::switchMaxDistanceDeactivate));
    listenStageSwitchOn(this, "SwitchFullVisibility3",
                        Functor(this, &ClippingAreaActorInfo::switchMaxDistanceActivate));
    listenStageSwitchOff(this, "SwitchFullVisibility3",
                         Functor(this, &ClippingAreaActorInfo::switchMaxDistanceDeactivate));
    listenStageSwitchOn(this, "SwitchFullVisibilityNoLOD",
                        Functor(this, &ClippingAreaActorInfo::switchMaxDistanceNoLODActivate));
    listenStageSwitchOff(this, "SwitchFullVisibilityNoLOD",
                         Functor(this, &ClippingAreaActorInfo::switchMaxDistanceNoLODDeactivate));
}

/**
 * Makes the group fully visible.
 */
void ClippingAreaActorInfo::switchMaxDistanceActivate() {
    mIsFullVisibility = true;
    mIsSwitchChanged = true;
}

/**
 * Stops making the group fully visible.
 */
void ClippingAreaActorInfo::switchMaxDistanceDeactivate() {
    mIsFullVisibility = false;
    mIsSwitchChanged = true;
}

/**
 * Makes the group fully visible and disables its LOD.
 */
void ClippingAreaActorInfo::switchMaxDistanceNoLODActivate() {
    mIsFullVisibility = true;
    mIsLODDisabled = true;
    mIsSwitchChanged = true;
}

/**
 * Stops making the group fully visible and enables its LOD again.
 */
void ClippingAreaActorInfo::switchMaxDistanceNoLODDeactivate() {
    mIsFullVisibility = false;
    mIsLODDisabled = false;
    mIsSwitchChanged = true;
}

/**
 * Decides how the group is clipped.
 */
void ClippingAreaActorInfo::setType() {
    if (mIsMaxClipping && mViewArea != nullptr) {
        mType = 0;
        mFadeState = 0;
        mAlpha = 1.0f;
    } else if ((mViewArea == nullptr && mViewFadeInAreas == nullptr) ||
               (mIsAlsoUseViewCtrlCulling && !mIsUseViewFadeInAreas)) {
        mType = 1;
    } else {
        mType = 2;
    }
}

/**
 * Constructs an empty node list.
 */
ClippingAreaActorInfoNodeList::ClippingAreaActorInfoNodeList() {
    initOffset(0);
}

/**
 * Constructs the clipping info of a single actor.
 * @param pNode node of the actor
 * @param rInfo actor init info
 * @param pPlacementInfo placement of the actor (may be null)
 */
ClippingAreaActorInfo::ClippingAreaActorInfo(ClippingAreaActorInfoNode* pNode,
                                             const ActorInitInfo& rInfo,
                                             const PlacementInfo* pPlacementInfo)
    : mPlacementId(nullptr) {
    if (pPlacementInfo != nullptr) {
        readClippingDistance(*pPlacementInfo);
        tryGetArg(&mIsMaxClipping, *pPlacementInfo, "MaxClipping");

        if (mIsMaxClipping) {
            mFarDistance = 200000.0f;
            mNearDistance = 200000.0f;
        }

        tryGetArg(&mIsUseClippingRadius, *pPlacementInfo, "UseClippingRadius");
        initStageSwitchKeeper(*pPlacementInfo, rInfo);

        if (isExistLinkChild(*pPlacementInfo, "ClipForceViewArea", 0)) {
            mClipForceViewArea =
                new ClipForceViewArea("ClipForceViewArea", *pPlacementInfo, rInfo);
        }
    } else {
        mStageSwitchKeeper = nullptr;
    }

    mTransPtr = getTransPtr(pNode->mActor);
    registerActor(pNode, false);
    setType();
}

/**
 * Adds an actor to the group.
 * @param pNode node of the actor
 * @param isInvalid whether the clipping of the actor is invalid
 * @return whether the group was empty
 */
bool ClippingAreaActorInfo::registerActor(ClippingAreaActorInfoNode* pNode, bool isInvalid) {
    bool isEmpty = mNodeList.size() == 0;
    pNode->mInfo = this;

    if (pNode->mIsUseOwnClipping) {
        mIsUseNodeClipping = true;
    }

    if (isInvalid) {
        pNode->mIsInvalidClipping = true;
    } else {
        pNode->mIsInvalidClipping = false;
        mNodeList.pushBack(pNode);
    }

    return isEmpty;
}

/**
 * Constructs the clipping info of a placed view group.
 * @param rPlacementInfo placement of the view group
 * @param rInfo actor init info
 */
ClippingAreaActorInfo::ClippingAreaActorInfo(const PlacementInfo& rPlacementInfo,
                                             const ActorInitInfo& rInfo) {
    mTransPtr = &mTrans;
    getTrans(&mTrans, rPlacementInfo);
    readClippingDistance(rPlacementInfo);
    tryGetArg(&mIsUseViewGroupPosForLOD, rPlacementInfo, "IsUseViewGroupPosForLOD");
    tryGetArg(&mIsJumpFlipLod, rPlacementInfo, "IsJumpFlipLod");
    tryGetArg(&mIsForceVisibilityInCutscene, rPlacementInfo, "ForceVisibilityInCutscene");
    tryGetArg(&mIsMaxClipping, rPlacementInfo, "MaxClipping");
    tryGetArg(&mIsUseFarLod, rPlacementInfo, "UseFarLod");
    tryGetArg(&mIsAlsoUseViewCtrlCulling, rPlacementInfo, "AlsoUseViewCtrlCulling");

    if (mIsMaxClipping && !mIsUseFarLod) {
        mNearDistance = 200000.0f;
        mFarDistance = 200000.0f;
    }

    mPlacementId = new PlacementId();
    mPlacementId->init(rPlacementInfo);

    if (isExistLinkChild(rPlacementInfo, "ViewFadeInAreas", 0)) {
        if (!mIsMaxClipping) {
            if (mIsAlsoUseViewCtrlCulling) {
                if (isExistLinkChild(rPlacementInfo, "ViewArea", 0)) {
                    initViewArea(rPlacementInfo, rInfo);
                } else {
                    mIsAlsoUseViewCtrlCulling = false;
                }
            }

            mViewFadeInAreas =
                new ClippingViewFadeInAreas("ViewFadeInAreas", rPlacementInfo, rInfo);
            mIsUseViewFadeInAreas = true;
        }
    } else if (isExistLinkChild(rPlacementInfo, "ViewArea", 0)) {
        initViewArea(rPlacementInfo, rInfo);

        if (!mViewArea->mIsBoundsValid) {
            mViewArea = nullptr;
        }
    }

    if (isExistLinkChild(rPlacementInfo, "ClipForceViewArea", 0)) {
        mClipForceViewArea = new ClipForceViewArea("ClipForceViewArea", rPlacementInfo, rInfo);
    }

    if (isExistLinkChild(rPlacementInfo, "FarLodArea", 0)) {
        ActorInitInfo farLodInfo;
        farLodInfo.initViewIdHost(&rPlacementInfo, rInfo);
        mFarLodAreas = new AreaObjGroup("FarLodArea", "FarLodArea", farLodInfo);
    }

    setType();
    initStageSwitchKeeper(rPlacementInfo, rInfo);
}

/**
 * Creates the stage switch keeper.
 */
void ClippingAreaActorInfo::initStageSwitchKeeper() {
    mStageSwitchKeeper = new StageSwitchKeeper();
}

/**
 * Removes an actor from the group.
 * @param pNode node of the actor
 * @return whether the group is now empty
 */
bool ClippingAreaActorInfo::removeActor(ClippingAreaActorInfoNode* pNode) {
    mNodeList.erase(pNode);
    pNode->mIsInvalidClipping = true;
    return mNodeList.size() == 0;
}

/**
 * Checks whether this group belongs to a placement.
 * @param rId placement id
 * @return whether the placement id matches
 */
bool ClippingAreaActorInfo::isViewInfo(const PlacementId& rId) const {
    return mPlacementId != nullptr && mPlacementId->isEqual(rId);
}

/**
 * Sets the near and far clipping distances.
 * @param near near distance
 * @param far far distance
 */
void ClippingAreaActorInfo::updateNarFarClip(f32 near, f32 far) {
    mNearDistance = near;
    mFarDistance = far;
}

/**
 * Sets the clipping radius, if the group uses it.
 * @param radius clipping radius
 */
void ClippingAreaActorInfo::setClippingRadius(f32 radius) {
    if (mIsUseClippingRadius) {
        mClippingRadius = radius;
    }
}

/**
 * Sets whether the group uses the expanded clipping mode, if it is visible in cutscenes.
 * @param isExpanded whether to expand the clipping
 */
void ClippingAreaActorInfo::setExpandedClippingMode(bool isExpanded) {
    if (mIsForceVisibilityInCutscene) {
        mIsExpandedClippingMode = isExpanded;
    }
}

/**
 * Disables the far LOD of the group.
 */
void ClippingAreaActorInfo::disableFarLod() {
    mIsUseFarLod = false;

    if (mIsMaxClipping) {
        mNearDistance = 200000.0f;
        mFarDistance = 200000.0f;
    }
}

/**
 * Gets the alpha step applied each frame while fading.
 * @return fade step
 */
f32 ClippingAreaActorInfo::getFadeStep() const {
    return 1.0f / 30.0f;
}

/**
 * Updates the fade state and alpha of the group.
 * @param distanceSq squared distance between the observer and the group
 * @return new alpha
 */
f32 ClippingAreaActorInfo::calculateAlpha(f32 distanceSq) {
    if (distanceSq >= 200000.0f * 200000.0f) {
        mFadeState = 1;
        mAlpha = 0.0f;
    } else if (mIsExpandedClippingMode) {
        mFadeState = 0;
        mAlpha = 1.0f;
    } else {
        if (mFadeState == 0) {
            mAlpha = 1.0f;

            if (distanceSq > mFarDistance * mFarDistance) {
                mFadeState = 3;
            }
        } else if (mFadeState == 3) {
            if (distanceSq <= mNearDistance * mNearDistance) {
                mFadeState = 2;
            } else {
                if (mIsSwitchChanged) {
                    mAlpha = 0.0f;
                }

                mAlpha -= 1.0f / 30.0f;

                if (mAlpha < 0.0f) {
                    mFadeState = 1;
                    mAlpha = 0.0f;
                }
            }
        } else if (mFadeState == 2) {
            if (distanceSq > mFarDistance * mFarDistance) {
                mFadeState = 3;
            } else {
                if (mIsSwitchChanged) {
                    mAlpha = 1.0f;
                }

                mAlpha += 1.0f / 30.0f;

                if (mAlpha >= 1.0f) {
                    mAlpha = 1.0f;
                    mFadeState = 0;
                }
            }
        } else {
            mAlpha = 0.0f;

            if (distanceSq <= mNearDistance * mNearDistance) {
                if (mIsSwitchChanged) {
                    mAlpha = 1.0f;
                    mFadeState = 0;
                } else {
                    mFadeState = 2;
                }
            }
        }
    }

    mIsSwitchChanged = false;
    return mAlpha;
}

/**
 * Hides every actor of the group and requests them to be clipped.
 * @param pHolder holder receiving clipping requests
 */
void ClippingAreaActorInfo::startClipped(ClippingAreaActorViewHolder* pHolder) {
    for (ClippingAreaActorInfoNode& node : mNodeList.robustRange()) {
        LiveActor* actor = node.mActor;
        actor->mGlobalAlpha = 0.0f;

        if (!isDead(actor)) {
            pHolder->mStartClippedRequests.pushBack(actor);
        }
    }
}

/**
 * Requests every actor of the group to stop being clipped.
 * @param pHolder holder receiving clipping requests
 */
void ClippingAreaActorInfo::endClipped(ClippingAreaActorViewHolder* pHolder) {
    for (ClippingAreaActorInfoNode& node : mNodeList.robustRange()) {
        LiveActor* actor = node.mActor;
        actor->mGlobalAlpha = 0.0f;
        pHolder->mEndClippedRequests.pushBack(actor);
    }
}

/**
 * Updates the LOD of the group if its first actor allows switching it.
 * @param pHolder holder receiving LOD requests
 */
void ClippingAreaActorInfo::updateJumpFlipLod(ClippingAreaActorViewHolder* pHolder) {
    if (mNodeList.size() != 0 && mNodeList.front()->mActor->isFarLodSwitchOkay()) {
        updateLod(pHolder);
    }
}

/**
 * Starts or ends the far LOD of the group depending on the observer position.
 * @param pHolder holder receiving LOD requests
 */
void ClippingAreaActorInfo::updateLod(ClippingAreaActorViewHolder* pHolder) {
    if (ClippingDirectorBase::sLODDisabled || mIsLODDisabled) {
        if (mIsInLod) {
            endLod(pHolder);
        }

        return;
    }

    ClippingJudge* judge = pHolder->mJudge;
    bool isFar;

    if (mFarLodAreas != nullptr) {
        isFar = !isInFarLodAreas(judge->mCameraPos);
    } else {
        f32 distanceSq;

        if (mIsUseViewGroupPosForLOD) {
            distanceSq = (judge->mCameraPos - *mTransPtr).squaredLength();
        } else {
            distanceSq = (judge->mCameraPos - mViewArea->mCenter).squaredLength();
        }

        isFar = distanceSq >= mFarDistance * mFarDistance;
    }

    if (isFar) {
        if (!mIsInLod) {
            startLod(pHolder);
        }
    } else if (mIsInLod) {
        endLod(pHolder);
    }
}

/**
 * Checks whether a position is inside one of the far LOD areas.
 * @param rPos position
 * @return whether the position is in a far LOD area
 */
bool ClippingAreaActorInfo::isInFarLodAreas(const sead::Vector3f& rPos) {
    for (s32 i = 0; i < mFarLodAreas->getSize(); i++) {
        if (mFarLodAreas->getAreaObj(i)->isInVolume(rPos)) {
            return true;
        }
    }

    return false;
}

/**
 * Ends the far LOD of every actor of the group.
 * @param pHolder holder receiving LOD requests
 */
void ClippingAreaActorInfo::endLod(ClippingAreaActorViewHolder* pHolder) {
    mIsInLod = false;

    for (ClippingAreaActorInfoNode& node : mNodeList.robustRange()) {
        LiveActor* actor = node.mActor;

        if (!isDead(actor)) {
            pHolder->mEndFarLodRequests.pushBack(actor);
        }
    }
}

/**
 * Starts the far LOD of every actor of the group.
 * @param pHolder holder receiving LOD requests
 */
void ClippingAreaActorInfo::startLod(ClippingAreaActorViewHolder* pHolder) {
    mIsInLod = true;

    for (ClippingAreaActorInfoNode& node : mNodeList.robustRange()) {
        LiveActor* actor = node.mActor;

        if (!isDead(actor)) {
            pHolder->mStartFarLodRequests.pushBack(actor);
        }
    }
}

/**
 * Checks whether a visible group has to be clipped and updates it.
 * @param pHolder holder receiving clipping requests
 * @return whether the group got clipped
 */
bool ClippingAreaActorInfo::checkClipping(ClippingAreaActorViewHolder* pHolder) {
    ClippingJudge* judge = pHolder->mJudge;
    bool isForceClip = false;

    if (mClipForceViewArea != nullptr && !pHolder->mIsForceClipAreasDisabled) {
        isForceClip = mClipForceViewArea->isInArea(judge->mPlayerPos);
    }

    if (pHolder->mForceClipArea != nullptr) {
        isForceClip |= !pHolder->isInForceClipViewCtrlArea(*mTransPtr);
    }

    if (mType == 0) {
        if (mIsUseFarLod) {
            if (mIsJumpFlipLod) {
                updateJumpFlipLod(pHolder);
            } else {
                updateLod(pHolder);
            }
        }

        updateShadowClipping(judge->mCameraPos);

        if (!pHolder->mIsCollisionClippingDisabled) {
            updateCollisionSettings(1.0f, judge->mPlayerPos, &mViewArea->mCenter,
                                    mViewArea->mRadius, true);
        }

        if (isForceClip || mViewArea->isClipped(judge)) {
            mClippingState = 1;
            startClipped(pHolder);
            return true;
        }

        return false;
    }

    if (mType == 1) {
        if (mIsFullVisibility) {
            calculateAlpha(0.0f);
        } else {
            calculateAlpha((judge->mCameraPos - *mTransPtr).squaredLength());
        }

        if (isForceClip || mFadeState == 1) {
            mClippingState = 1;
            mCollisionState = 1;

            for (ClippingAreaActorInfoNode& node : mNodeList.robustRange()) {
                LiveActor* actor = node.mActor;

                if (node.mIsCollisionEnabled) {
                    if (!node.mIsNoCollisionClip) {
                        disableAllCollisionParts(actor);
                    }

                    node.mIsCollisionEnabled = false;
                }

                if (node.mShadowClippingDistance <= 0.0f) {
                    setDisableDepthShadow(actor, true, false);
                }

                actor->mGlobalAlpha = 0.0f;
            }

            startClipped(pHolder);
            return true;
        }

        ClippingPosInfo posInfo(this);

        if (mIsMaxClipping) {
            if (!pHolder->mIsCollisionClippingDisabled) {
                updateCollisionSettings(mAlpha, judge->mPlayerPos, posInfo.mPos, posInfo.mRadius,
                                        true);
            }
        } else {
            if (!pHolder->mIsCollisionClippingDisabled) {
                updateOnlyCollisionSettings(judge->mPlayerPos, posInfo.mPos, posInfo.mRadius,
                                            true);
            }

            if (mViewArea != nullptr) {
                if (mViewArea->isClipped(judge)) {
                    mClippingState = 1;
                    startClipped(pHolder);
                    return true;
                }
            } else if (mClippingRadius > 0.0f &&
                       judge->isJudgedToClipFrustum(*mTransPtr, mClippingRadius, 200.0f, 1)) {
                mClippingState = 1;
                startClipped(pHolder);
                return true;
            }

            updateAlphaOnList(mAlpha, pHolder);
        }

        updateShadowClipping(judge->mCameraPos);
        return false;
    }

    ClippingPosInfo posInfo(this);
    f32 alpha;

    if (mIsUseViewFadeInAreas) {
        if (mIsExpandedClippingMode || mIsFullVisibility) {
            mIsSwitchChanged = false;
            alpha = 1.0f;
        } else {
            alpha = mViewFadeInAreas->updateClipping(judge->mPlayerPos, mIsSwitchChanged);
            mIsSwitchChanged = false;
        }

        if (alpha != 0.0f && mViewArea != nullptr && mViewArea->isClipped(judge)) {
            if (!pHolder->mIsCollisionClippingDisabled) {
                updateOnlyCollisionSettings(judge->mPlayerPos, posInfo.mPos, posInfo.mRadius,
                                            true);
            }

            for (ClippingAreaActorInfoNode& node : mNodeList.robustRange()) {
                node.mActor->mGlobalAlpha = alpha;
            }

            mClippingState = 1;
            startClipped(pHolder);
            return true;
        }
    } else {
        alpha = 1.0f;

        if (!mIsExpandedClippingMode && !mIsFullVisibility) {
            alpha = mViewArea->updateClipping(judge->mPlayerPos, mIsSwitchChanged);
        }

        mIsSwitchChanged = false;
    }

    if (alpha == 0.0f || isForceClip) {
        mClippingState = 1;

        if (mCollisionState == 0) {
            mCollisionState = 1;

            for (ClippingAreaActorInfoNode& node : mNodeList.robustRange()) {
                LiveActor* actor = node.mActor;

                if (node.mIsCollisionEnabled) {
                    if (!node.mIsNoCollisionClip) {
                        disableAllCollisionParts(actor);
                    }

                    node.mIsCollisionEnabled = false;
                }

                if (node.mShadowClippingDistance <= 0.0f) {
                    setDisableDepthShadow(actor, true, false);
                }

                actor->mGlobalAlpha = alpha;
            }
        }

        startClipped(pHolder);
        return true;
    }

    updateShadowClipping(judge->mCameraPos);

    if (!pHolder->mIsCollisionClippingDisabled) {
        updateOnlyCollisionSettings(judge->mPlayerPos, posInfo.mPos, posInfo.mRadius, true);
    }

    updateAlphaOnList(alpha, pHolder);
    return false;
}

/**
 * Checks whether a clipped group has to become visible again and updates it.
 * @param pHolder holder receiving clipping requests
 * @return whether the group stopped being clipped
 */
bool ClippingAreaActorInfo::checkStillClipping(ClippingAreaActorViewHolder* pHolder) {
    ClippingJudge* judge = pHolder->mJudge;
    bool isForceClip = false;

    if (mClipForceViewArea != nullptr && !pHolder->mIsForceClipAreasDisabled) {
        isForceClip = mClipForceViewArea->isInArea(judge->mPlayerPos);
    }

    if (pHolder->mForceClipArea != nullptr) {
        isForceClip |= !pHolder->isInForceClipViewCtrlArea(*mTransPtr);
    }

    if (mType == 0) {
        if (mIsUseFarLod) {
            if (mIsJumpFlipLod) {
                updateJumpFlipLod(pHolder);
            } else {
                updateLod(pHolder);
            }
        }

        if (!isForceClip && !mViewArea->isClipped(judge)) {
            mClippingState = 0;
            mCollisionState = 0;
            endClipped(pHolder);

            for (ClippingAreaActorInfoNode& node : mNodeList.robustRange()) {
                LiveActor* actor = node.mActor;

                if (!node.mIsCollisionEnabled) {
                    enableAllCollisionParts(actor);
                    node.mIsCollisionEnabled = true;
                }

                if (node.mShadowClippingDistance <= 0.0f) {
                    setDisableDepthShadow(actor, false, false);
                }

                actor->mGlobalAlpha = 1.0f;
            }

            updateAlphaOnList(1.0f, pHolder);
            return true;
        }

        if (!pHolder->mIsCollisionClippingDisabled) {
            updateCollisionSettings(1.0f, judge->mPlayerPos, &mViewArea->mCenter,
                                    mViewArea->mRadius, false);
        }

        return false;
    }

    if (mType == 1) {
        if (mIsFullVisibility) {
            calculateAlpha(0.0f);
        } else {
            calculateAlpha((judge->mCameraPos - *mTransPtr).squaredLength());
        }

        ClippingPosInfo posInfo(this);

        if (isForceClip || mFadeState == 1) {
            if (mCollisionState == 0) {
                mCollisionState = 1;

                for (ClippingAreaActorInfoNode& node : mNodeList.robustRange()) {
                    LiveActor* actor = node.mActor;

                    if (node.mIsCollisionEnabled) {
                        if (!node.mIsNoCollisionClip) {
                            disableAllCollisionParts(actor);
                        }

                        node.mIsCollisionEnabled = false;
                    }

                    if (node.mShadowClippingDistance <= 0.0f) {
                        setDisableDepthShadow(actor, true, false);
                    }

                    actor->mGlobalAlpha = 0.0f;
                }
            }

            return false;
        }

        bool isClipped;

        if (mViewArea != nullptr) {
            isClipped = mViewArea->isClipped(judge);
        } else {
            isClipped = mClippingRadius > 0.0f &&
                        judge->isJudgedToClipFrustum(*mTransPtr, mClippingRadius, 200.0f, 1);
        }

        if (isClipped) {
            if (!pHolder->mIsCollisionClippingDisabled) {
                updateOnlyCollisionSettings(judge->mPlayerPos, posInfo.mPos, posInfo.mRadius,
                                            true);
            }

            return false;
        }

        mClippingState = 0;
        endClipped(pHolder);

        if (!pHolder->mIsCollisionClippingDisabled) {
            updateOnlyCollisionSettings(judge->mPlayerPos, posInfo.mPos, posInfo.mRadius, true);
        }

        updateAlphaOnList(mAlpha, pHolder);
        return true;
    }

    ClippingPosInfo posInfo(this);

    if (!pHolder->mIsCollisionClippingDisabled) {
        updateOnlyCollisionSettings(judge->mPlayerPos, posInfo.mPos, posInfo.mRadius, true);
    }

    f32 alpha;

    if (mIsUseViewFadeInAreas) {
        if (mIsExpandedClippingMode || mIsFullVisibility) {
            mIsSwitchChanged = false;
            alpha = 1.0f;
        } else {
            alpha = mViewFadeInAreas->updateClipping(judge->mPlayerPos, mIsSwitchChanged);
            mIsSwitchChanged = false;
        }

        if (alpha != 0.0f && mViewArea != nullptr && mViewArea->isClipped(judge)) {
            for (ClippingAreaActorInfoNode& node : mNodeList.robustRange()) {
                node.mActor->mGlobalAlpha = alpha;
            }

            return false;
        }
    } else {
        alpha = 1.0f;

        if (!mIsExpandedClippingMode && !mIsFullVisibility) {
            alpha = mViewArea->updateClipping(judge->mPlayerPos, mIsSwitchChanged);
        }

        mIsSwitchChanged = false;
    }

    if (alpha == 0.0f || isForceClip) {
        return false;
    }

    mClippingState = 0;
    endClipped(pHolder);
    updateAlphaOnList(alpha, pHolder);
    return true;
}

/**
 * Gets the clipping position and radius of a group.
 * @param pInfo clipping info of the group
 */
ClippingAreaActorInfo::ClippingPosInfo::ClippingPosInfo(ClippingAreaActorInfo* pInfo) {
    if (pInfo->mViewArea != nullptr) {
        mRadius = pInfo->mViewArea->mRadius;
        mPos = &pInfo->mViewArea->mCenter;
    } else {
        mPos = pInfo->mTransPtr;

        if (pInfo->mClippingRadius > 0.0f) {
            mRadius = pInfo->mClippingRadius;
        } else {
            mRadius = 0.0f;
        }
    }
}
}  // namespace al

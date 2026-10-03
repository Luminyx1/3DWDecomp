#pragma once

#include <basis/seadTypes.h>
#include <container/seadOffsetList.h>
#include <math/seadVector.h>

#include "Library/StageSwitch/Core/IUseStageSwitch.hpp"

namespace al {
class ActorInitInfo;
class AreaObjGroup;
class ClipForceViewArea;
class ClippingAreaActorInfo;
class ClippingAreaActorViewArea;
class ClippingAreaActorViewHolder;
class ClippingViewFadeInAreas;
class LiveActor;
class PlacementId;
class PlacementInfo;
class StageSwitchKeeper;

/**
 * Per-actor entry of a ClippingAreaActorInfo (one clipping "view group").
 */
struct ClippingAreaActorInfoNode {
    ClippingAreaActorInfoNode()
        : mActor(nullptr), mInfo(nullptr), mIsInvalidClipping(true), mIsCollisionEnabled(true),
          mIsShadowVisible(true), mIsUseOwnClipping(false), _24(true), mIsDrawVisible(true),
          mIsNoCollisionClip(false), mRequestType(0), mClippingRadius(-1.0f),
          mDrawClippingRadius(-1.0f), mClippingOffset(sead::Vector3f::zero),
          mShadowClippingDistance(0.0f) {}

    sead::ListNode mListNode;
    LiveActor* mActor;
    ClippingAreaActorInfo* mInfo;
    bool mIsInvalidClipping;
    bool mIsCollisionEnabled;
    bool mIsShadowVisible;
    bool mIsUseOwnClipping;
    bool _24;
    bool mIsDrawVisible;
    bool mIsNoCollisionClip;
    u8 _27;
    union {
        f32 _28;
        s32 mRequestType;  // pending ClippingAreaActorViewHolder request (0 none, 1 remove, 2 add)
    };
    f32 mClippingRadius;
    f32 mDrawClippingRadius;
    sead::Vector3f mClippingOffset;
    f32 mShadowClippingDistance;
    u8 _44[0x48 - 0x44];
};

static_assert(sizeof(ClippingAreaActorInfoNode) == 0x48);

/**
 * Intrusive list of ClippingAreaActorInfoNode.
 */
class ClippingAreaActorInfoNodeList : public sead::OffsetList<ClippingAreaActorInfoNode> {
public:
    ClippingAreaActorInfoNodeList();
};

/**
 * Clipping state shared by all actors of one clipping view group.
 */
class ClippingAreaActorInfo : public IUseStageSwitch {
public:
    /**
     * Position and radius used to clip the group.
     */
    struct ClippingPosInfo {
        ClippingPosInfo(ClippingAreaActorInfo* pInfo);

        const sead::Vector3f* mPos;
        f32 mRadius;
    };

    ClippingAreaActorInfo(ClippingAreaActorInfoNode* pNode, const ActorInitInfo& rInfo,
                          const PlacementInfo* pPlacementInfo);
    ClippingAreaActorInfo(const PlacementInfo& rPlacementInfo, const ActorInitInfo& rInfo);

    const char* getName() const override { return "ClippingAreaActorInfo"; }

    StageSwitchKeeper* getStageSwitchKeeper() const override { return mStageSwitchKeeper; }

    void initStageSwitchKeeper() override;

    void updateAlphaOnList(f32 alpha, ClippingAreaActorViewHolder* pHolder);
    void updateOnlyCollisionSettings(const sead::Vector3f& rPos, const sead::Vector3f* pCenter,
                                     f32 radius, bool isUnused);
    void updateCollisionSettings(f32 alpha, const sead::Vector3f& rPos,
                                 const sead::Vector3f* pCenter, f32 radius, bool isUnused);
    void updateShadowClipping(const sead::Vector3f& rPos);
    void readClippingDistance(const PlacementInfo& rPlacementInfo);
    bool initViewArea(const PlacementInfo& rPlacementInfo, const ActorInitInfo& rInfo);
    void initStageSwitchKeeper(const PlacementInfo& rPlacementInfo, const ActorInitInfo& rInfo);
    void switchMaxDistanceActivate();
    void switchMaxDistanceDeactivate();
    void switchMaxDistanceNoLODActivate();
    void switchMaxDistanceNoLODDeactivate();
    void setType();
    bool registerActor(ClippingAreaActorInfoNode* pNode, bool isInvalid);
    bool removeActor(ClippingAreaActorInfoNode* pNode);
    bool isViewInfo(const PlacementId& rId) const;
    void updateNarFarClip(f32 near, f32 far);
    void setClippingRadius(f32 radius);
    void setExpandedClippingMode(bool isExpanded);
    void disableFarLod();
    f32 getFadeStep() const;
    f32 calculateAlpha(f32 distanceSq);
    void startClipped(ClippingAreaActorViewHolder* pHolder);
    void endClipped(ClippingAreaActorViewHolder* pHolder);
    void updateJumpFlipLod(ClippingAreaActorViewHolder* pHolder);
    void updateLod(ClippingAreaActorViewHolder* pHolder);
    bool isInFarLodAreas(const sead::Vector3f& rPos);
    void endLod(ClippingAreaActorViewHolder* pHolder);
    void startLod(ClippingAreaActorViewHolder* pHolder);
    bool checkClipping(ClippingAreaActorViewHolder* pHolder);
    bool checkStillClipping(ClippingAreaActorViewHolder* pHolder);

    bool isExpandedClippingMode() const { return mIsExpandedClippingMode; }

    void* _8 = nullptr;
    void* _10 = nullptr;
    StageSwitchKeeper* mStageSwitchKeeper = nullptr;
    ClippingAreaActorInfoNodeList mNodeList;
    PlacementId* mPlacementId;
    f32 mNearDistance = 7500.0f;
    f32 mFarDistance = 8000.0f;
    sead::Vector3f mTrans;
    const sead::Vector3f* mTransPtr;
    ClippingAreaActorViewArea* mViewArea = nullptr;
    ClippingViewFadeInAreas* mViewFadeInAreas = nullptr;
    ClipForceViewArea* mClipForceViewArea = nullptr;
    AreaObjGroup* mFarLodAreas = nullptr;
    s32 mClippingState = 2;
    s32 mFadeState = 1;
    s32 mCollisionState = 0;
    bool mIsMaxClipping = false;
    bool mIsUseFarLod = false;
    bool mIsInLod = false;
    bool mIsAlsoUseViewCtrlCulling = false;
    bool mIsUseViewFadeInAreas = false;
    bool mIsUseClippingRadius = false;
    bool mIsUseNodeClipping = false;
    bool mIsSwitchChanged = false;
    bool mIsForceVisibilityInCutscene = false;
    bool mIsExpandedClippingMode = false;
    bool mIsUseViewGroupPosForLOD = false;
    s32 mType = 0;
    f32 mClippingRadius = -1.0f;
    f32 mAlpha = 0.0f;
    bool mIsJumpFlipLod = false;
    bool mIsFullVisibility = false;
    bool mIsLODDisabled = false;
    bool mIsShadowClipping = false;
};

static_assert(sizeof(ClippingAreaActorInfo) == 0xa8);
}  // namespace al

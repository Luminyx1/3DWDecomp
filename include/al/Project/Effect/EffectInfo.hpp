#pragma once

#include <gfx/seadColor.h>
#include <math/seadVector.h>

#include <basis/seadTypes.h>

#include "Project/Effect/EffectDataBase.hpp"

namespace al {
struct EffectResourceInfo;

struct EffectEmitInfo {
    EffectEmitInfo();

    s32 mGroupId;
    const char* mJointName;
    sead::Vector3f mOffsetTrans;
    sead::Vector3f mOffsetRotate;
    f32 mScale;
    f32 mParticleScale;
    f32 mEmitRatio;
    f32 mFarClipDistance;
    s32 mForceCalcFrame;
    s32 mHandleNum;
    sead::Color4f mColor;
    bool mIsBillboard;
    bool mIsYBillboard;
    bool mIsFollowCamera;
    bool mIsReEmitOnClip;
    bool mIsOneTimeFade;
    bool mIsEmitIgnoreRotate;
    bool mIsEmitIgnoreScale;
    bool mIsFollowPos;
    bool mIsFollowMtx;
    bool mIsFollowTransOnEmit;
    bool mIsFollowRotateOnEmit;
    bool mIsFollowScaleOnEmit;
    bool mIsFollowTrans;
    bool mIsFollowRotate;
    bool mIsFollowScale;
    bool mIsNeedProgramInfo;
    mutable bool mIsSetPosPtr;
    mutable bool mIsSetMtxPtr;
    bool mIsIgnoreJoint;
    bool mIsAddOffsetTrans;
    bool mIsAddOffsetRotate;
    bool mIsSetColor;
    bool mIsKeepFrontX;
    bool mIsKeepFrontY;
    bool mIsKeepFrontZ;
    bool mIsNoEmitAtNoCollide;
    bool mIsFollowCameraFovy;
    bool mIsSnapshotCameraMode;
};

static_assert(sizeof(EffectEmitInfo) == 0x70);

using EffectEmitParam = EffectEmitInfo;

struct ActionEffectData {
    ActionEffectData();

    const char* mActionName;
    s32 mStartFrame;
    s32 mEndFrame;
    bool mIsKeepEmitter;
    void* _18;
};

static_assert(sizeof(ActionEffectData) == 0x20);

struct EffectHitReactionData {
    EffectHitReactionData();

    const char* mEffectName;
    f32 mPosOffsetBetweenSensors;
};

static_assert(sizeof(EffectHitReactionData) == 0x10);

struct EffectHitReactionInfo {
    EffectHitReactionInfo();

    const char* mReactionName;
    s32 mDataNum;
    EffectHitReactionData* mDatas;
};

static_assert(sizeof(EffectHitReactionInfo) == 0x18);

struct EffectInfo {
    EffectInfo();

    const char* mName;
    s32 mEmitInfoNum;
    EffectResourceInfo* mEmitInfos;
    EffectEmitInfo mParam;
    s32 mActionNum;
    ActionEffectData* mActions;
    s32 mHitReactionNum;
};

static_assert(sizeof(EffectInfo) == 0xa0);

struct EffectNamedMtxList {
    const char** mNames;
    s32 mNum;
};

struct EffectUserInfo {
    EffectUserInfo();

    EffectInfo* tryFindEffectInfo(const char* pName) const;
    EffectHitReactionInfo* tryFindHitReactionInfo(const char* pName) const;

    const char* mName;
    s32 mEffectNum;
    EffectInfo* mEffects;
    s32 mHitReactionNum;
    EffectHitReactionInfo* mHitReactions;
    EffectNamedMtxList* mNamedMtxList;
};

static_assert(sizeof(EffectUserInfo) == 0x30);

}  // namespace al

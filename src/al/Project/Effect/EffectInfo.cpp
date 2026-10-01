#include "Project/Effect/EffectInfo.hpp"

#include "Library/Effect/EmitterSetResourceInfoHolder.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Effect/EffectEmitter.hpp"

namespace al {

/**
 * Constructs emit settings with default values.
 */
EffectEmitInfo::EffectEmitInfo()
    : mGroupId(0), mJointName(nullptr), mOffsetTrans(0.0f, 0.0f, 0.0f),
      mOffsetRotate(0.0f, 0.0f, 0.0f), mScale(1.0f), mParticleScale(1.0f), mEmitRatio(1.0f),
      mFarClipDistance(0.0f), mForceCalcFrame(0), mHandleNum(1), mColor(sead::Color4f::cWhite),
      mIsBillboard(false), mIsYBillboard(false), mIsFollowCamera(false), mIsReEmitOnClip(true),
      mIsOneTimeFade(false), mIsEmitIgnoreRotate(false), mIsEmitIgnoreScale(false),
      mIsFollowPos(false), mIsFollowMtx(false), mIsFollowTransOnEmit(true),
      mIsFollowRotateOnEmit(true), mIsFollowScaleOnEmit(false), mIsFollowTrans(false),
      mIsFollowRotate(false), mIsFollowScale(false), mIsNeedProgramInfo(false),
      mIsSetPosPtr(false), mIsSetMtxPtr(false), mIsIgnoreJoint(false), mIsAddOffsetTrans(false),
      mIsAddOffsetRotate(false), mIsSetColor(false), mIsKeepFrontX(false), mIsKeepFrontY(false),
      mIsKeepFrontZ(false), mIsNoEmitAtNoCollide(false), mIsFollowCameraFovy(false),
      mIsSnapshotCameraMode(false) {}

/**
 * Constructs an action bound effect entry.
 */
ActionEffectData::ActionEffectData()
    : mActionName(nullptr), mStartFrame(0), mEndFrame(-1), mIsKeepEmitter(false), _18(nullptr) {}

/**
 * Constructs an empty hit reaction effect entry.
 */
EffectHitReactionData::EffectHitReactionData()
    : mEffectName(nullptr), mPosOffsetBetweenSensors(0.0f) {}

/**
 * Constructs an empty hit reaction.
 */
EffectHitReactionInfo::EffectHitReactionInfo()
    : mReactionName(nullptr), mDataNum(0), mDatas(nullptr) {}

/**
 * Constructs an empty particle resource reference.
 */
EffectResourceInfo::EffectResourceInfo()
    : mName(nullptr), mMaterialName(""), _10(""), _18(""), mJointName(nullptr),
      mEmitterSetResourceInfo(&EmitterSetResourceInfo::InvalidResource) {}

/**
 * Constructs an effect with default settings.
 */
EffectInfo::EffectInfo()
    : mName(nullptr), mEmitInfoNum(0), mEmitInfos(nullptr), mActionNum(0), mActions(nullptr),
      mHitReactionNum(0) {}

/**
 * Constructs an empty effect user.
 */
EffectUserInfo::EffectUserInfo()
    : mName(nullptr), mEffectNum(0), mEffects(nullptr), mHitReactionNum(0),
      mHitReactions(nullptr), mNamedMtxList(nullptr) {}

/**
 * Finds an effect by name.
 * @param pName Effect name.
 * @return The effect, or null if not found.
 */
EffectInfo* EffectUserInfo::tryFindEffectInfo(const char* pName) const {
    for (s32 i = 0; i < mEffectNum; i++) {
        EffectInfo* info = &mEffects[i];

        if (isEqualString(info->mName, pName)) {
            return info;
        }
    }

    return nullptr;
}

/**
 * Finds a hit reaction by name.
 * @param pName Hit reaction name.
 * @return The hit reaction, or null if not found.
 */
EffectHitReactionInfo* EffectUserInfo::tryFindHitReactionInfo(const char* pName) const {
    for (s32 i = 0; i < mHitReactionNum; i++) {
        EffectHitReactionInfo* info = &mHitReactions[i];

        if (isEqualString(info->mReactionName, pName)) {
            return info;
        }
    }

    return nullptr;
}

}  // namespace al

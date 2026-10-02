#include "Library/Anim/AnimPlayerSkl.hpp"

#include <arm_neon.h>
#include <cstring>
#include <math/seadVector.h>
#include <nn/g3d/g3d_ModelObj.h>
#include <nn/g3d/g3d_SkeletalAnimObj.h>
#include <nn/g3d/g3d_SkeletonObj.h>

#include "Library/Anim/ModelAnimInterp.hpp"
#include "Library/Anim/SklAnimRetargettingInfo.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Project/Anim/AnimInfo.hpp"
#include "Project/Anim/AnimPlayerSimple.hpp"
#include "Project/Anim/InitResourceDataAnim.hpp"

namespace {

/**
 * Stores the first three components of a vector.
 * @param pOut Vector receiving the components.
 * @param vec Source vector.
 */
inline void storeVectorSimd(sead::Vector3f* pOut, float32x4_t vec) {
    vst1_f32(&pOut->x, vget_low_f32(vec));
    vst1q_lane_f32(&pOut->z, vec, 2);
}

/**
 * Loads a vector with a zero fourth component.
 * @param rVec Source vector.
 * @return Loaded vector.
 */
inline float32x4_t loadVectorSimd(const sead::Vector3f& rVec) {
    float32x2_t low = vld1_f32(&rVec.x);
    float32x2_t high = vcreate_f32(*reinterpret_cast<const u32*>(&rVec.z));
    return vcombine_f32(low, high);
}

/**
 * Builds a vector with a zero fourth component.
 * @param x X component.
 * @param y Y component.
 * @param z Z component.
 * @return Vector.
 */
inline float32x4_t makeVectorSimd(f32 x, f32 y, f32 z) {
    float32x4_t vec = vdupq_n_f32(0.0f);
    vec = vsetq_lane_f32(x, vec, 0);
    vec = vsetq_lane_f32(y, vec, 1);
    return vsetq_lane_f32(z, vec, 2);
}

/**
 * Normalizes a vector, returning zero for a zero vector.
 * @param vec Source vector.
 * @return Normalized vector.
 */
inline float32x4_t normalizeVectorSimd(float32x4_t vec) {
    float32x4_t sq = vmulq_f32(vec, vec);
    float32x2_t sum = vadd_f32(vget_high_f32(sq), vget_low_f32(sq));
    sum = vpadd_f32(sum, sum);
    float32x4_t lengthSq = vcombine_f32(sum, sum);
    float32x4_t rsqrt = vrsqrteq_f32(lengthSq);
    rsqrt = vmulq_f32(rsqrt, vrsqrtsq_f32(rsqrt, vmulq_f32(rsqrt, lengthSq)));
    rsqrt = vmulq_f32(rsqrt, vrsqrtsq_f32(rsqrt, vmulq_f32(lengthSq, rsqrt)));
    uint32x4_t isNonZero = vmvnq_u32(vceqzq_f32(lengthSq));
    return vreinterpretq_f32_u32(
        vandq_u32(vreinterpretq_u32_f32(vmulq_f32(vec, rsqrt)), isNonZero));
}

/**
 * Counts a joint and all of its descendants.
 * @param pSkeleton Skeleton holding the joints.
 * @param jointIndex Index of the root joint.
 * @return Number of joints in the hierarchy below and including the root joint.
 */
s32 countJointsRecursive(const nn::g3d::SkeletonObj* pSkeleton, s32 jointIndex) {
    s32 count = 1;
    for (s32 i = 0; i < pSkeleton->GetBoneCount(); i++) {
        if (pSkeleton->GetBone(i)->GetParentIndex() == jointIndex) {
            count += countJointsRecursive(pSkeleton, i);
        }
    }

    return count;
}

/**
 * Adds a joint and all of its descendants to a joint index array.
 * @param pJoints Array receiving the joint indices.
 * @param pSkeleton Skeleton holding the joints.
 * @param jointIndex Index of the root joint.
 */
void addJointRecursive(al::AnimPlayerSkl::JointIndexArray* pJoints,
                       const nn::g3d::SkeletonObj* pSkeleton, s32 jointIndex) {
    pJoints->emplaceBack(jointIndex);

    for (s32 i = 0; i < pSkeleton->GetBoneCount(); i++) {
        if (pSkeleton->GetBone(i)->GetParentIndex() == jointIndex) {
            addJointRecursive(pJoints, pSkeleton, i);
        }
    }
}

}  // namespace

namespace al {

/**
 * Creates a skeletal animation player if the model has skeletal animations.
 * @param pInfo Animation player init info.
 * @param blendNum Maximum number of blended animations.
 * @return The new player, or nullptr if there are no skeletal animations.
 */
AnimPlayerSkl* AnimPlayerSkl::tryCreate(const AnimPlayerInitInfo* pInfo, s32 blendNum) {
    if (pInfo->mInitResourceDataAnim == nullptr ||
        pInfo->mInitResourceDataAnim->getSklAnimInfoTable() == nullptr) {
        return nullptr;
    }

    return new AnimPlayerSkl(pInfo, blendNum);
}

/**
 * Clears the per-frame flags at the end of a frame.
 */
void AnimPlayerSkl::updateLast() {
    AnimPlayerBase::updateLast();

    for (u32 i = 0; i < mPartialAnimInfos.size(); i++) {
        mPartialAnimInfos[i].isFrameSet = false;
    }
}

/**
 * Starts a skeletal animation, optionally blended with up to five more animations.
 * @param pInterpAnimName Name of the animation to interpolate from.
 * @param pName0 Name of the main animation.
 * @param pName1 Name of the first blended animation, or nullptr.
 * @param pName2 Name of the second blended animation, or nullptr.
 * @param pName3 Name of the third blended animation, or nullptr.
 * @param pName4 Name of the fourth blended animation, or nullptr.
 * @param pName5 Name of the fifth blended animation, or nullptr.
 */
void AnimPlayerSkl::startSklAnim(const char* pInterpAnimName, const char* pName0,
                                 const char* pName1, const char* pName2, const char* pName3,
                                 const char* pName4, const char* pName5) {
    clearSklAnimBlend();
    setSklAnim(pName0, 0);

    if (pName1 != nullptr) {
        setSklAnim(pName1, 1);
    }

    if (pName2 != nullptr) {
        setSklAnim(pName2, 2);
    }

    if (pName3 != nullptr) {
        setSklAnim(pName3, 3);
    }

    if (pName4 != nullptr) {
        setSklAnim(pName4, 4);
    }

    if (pName5 != nullptr) {
        setSklAnim(pName5, 5);
    }

    if (!mIsSkipInterp) {
        mModelAnimInterp->prepareAnimInterp(mModelObj->GetSkeleton(), pName0, pInterpAnimName,
                                            mAnimObjs);
    }

    _10 = true;
    _11 = true;
    mIsSkipInterp = false;
    calcSklAnim();
}

/**
 * Removes all blended animations, leaving only the main animation at full weight.
 */
void AnimPlayerSkl::clearSklAnimBlend() {
    for (s32 i = 1; i < mAnimObjs.size(); i++) {
        mAnimObjs[i] = nullptr;
        mBlendWeights[i] = 0.0f;
    }

    mBlendWeights[0] = 1.0f;
}

/**
 * Sets the animation of a blend slot and restarts it.
 * @param pName Animation name.
 * @param index Blend slot index.
 */
void AnimPlayerSkl::setSklAnim(const char* pName, s32 index) {
    const AnimResInfo* info = mInfoTable->findAnimInfo(pName);
    mAnimObjStack[index]->SetResource(static_cast<const nn::g3d::ResSkeletalAnim*>(info->resAnim));
    mAnimObjStack[index]->Bind(mModelObj->GetSkeleton()->GetRes());
    mAnimObjs[index] = mAnimObjStack[index];

    nn::g3d::SkeletalAnimObj* animObj = getAnimObj(index);
    animObj->GetFrameCtrl().SetFrame(0.0f);
    animObj->GetFrameCtrl().SetStep(1.0f);
}

/**
 * Calculates the animations and applies them to the skeleton.
 */
void AnimPlayerSkl::calcSklAnim() {
    bool isBlending = isBlend();
    bool isPartial = isAttachedPartialAnim();

    if (!isBlending && !isPartial && !mIsRetargettingValid) {
        nn::g3d::SkeletalAnimObj* animObj = mAnimObjs[0];
        if (animObj != nullptr) {
            animObj->Calculate();
            mAnimObjs[0]->ApplyTo(mModelObj->GetSkeleton());
        }
    } else {
        nn::g3d::SkeletalAnimBlender* blender = mBlender;
        nn::g3d::SkeletalAnimObj* mainAnimObj = mAnimObjs[0];
        blender->ClearResult();

        bool isApply = false;
        if (mainAnimObj != nullptr) {
            for (s32 i = 0; i < mAnimObjs.size(); i++) {
                if (mAnimObjs[i] != nullptr) {
                    blender->Blend(mAnimObjs[i], mBlendWeights[i]);
                }
            }

            if (mIsRetargettingValid && mRetargettingInfo != nullptr) {
                const SklAnimRetargettingInfo* info = mRetargettingInfo;
                nn::g3d::SkeletalAnimBlender* retargetBlender = mBlender;
                s32 boneNum = retargetBlender->GetBoneCount();
                for (s32 i = 0; i < boneNum; i++) {
                    const SklAnimRetargettingInfo::Entry& entry = info->getEntry(i);
                    if (entry.targetIndex == -1) {
                        continue;
                    }

                    float32x4_t& translate = retargetBlender->GetResult()[i].translate._v;
                    sead::Vector3f dir;
                    storeVectorSimd(&dir, translate);
                    if (!tryNormalizeOrZero(&dir)) {
                        continue;
                    }

                    sead::Vector3f trans(vgetq_lane_f32(translate, 0),
                                         vgetq_lane_f32(translate, 1),
                                         vgetq_lane_f32(translate, 2));
                    const sead::Vector3f& scale = info->getScale();
                    sead::Vector3f diff = trans - dir * entry.targetLength;
                    diff.x *= scale.x;
                    diff.y *= scale.y;
                    diff.z *= scale.z;
                    sead::Vector3f result = diff + dir * entry.length;
                    retargetBlender->GetResult()[i].translate._v =
                        makeVectorSimd(result.x, result.y, result.z);
                }
            }

            isApply = true;
        }

        if (isPartial) {
            nn::g3d::SkeletalAnimBlender* partialBlender = mBlender;
            for (s32 i = 0; i < mPartialAnimInfos.size(); i++) {
                PartialAnimInfo& partial = mPartialAnimInfos[i];
                if (!partial.isAttached) {
                    continue;
                }

                nn::g3d::SkeletalAnimObj* animObj = partial.animObj;
                for (s32 j = 0; j < partialBlender->GetMaxBoneCount(); j++) {
                    animObj->SetBindFlagImpl(j, nn::g3d::AnimObj::BindFlag_SkipApply);
                }

                JointIndexArray* joints = partial.joints;
                for (auto it = joints->begin(), end = joints->end(); it != end; ++it) {
                    partialBlender->GetResult()[*it] = {};
                    animObj->SetBindFlagImpl(*it, nn::g3d::AnimObj::BindFlag_None);
                }

                partialBlender->Blend(animObj, 1.0f);

                for (s32 j = 0; j < partialBlender->GetMaxBoneCount(); j++) {
                    animObj->SetBindFlagImpl(j, nn::g3d::AnimObj::BindFlag_None);
                }
            }

            nn::g3d::SkeletalAnimBlender* retargetBlender = mBlender;
            for (s32 i = 0; i < mPartialAnimInfos.size(); i++) {
                PartialAnimInfo& partial = mPartialAnimInfos[i];
                if (!partial.isAttached || partial.retargettingInfo == nullptr) {
                    continue;
                }

                for (s32& joint : *partial.joints) {
                    const SklAnimRetargettingInfo* info = partial.retargettingInfo;
                    const SklAnimRetargettingInfo::Entry& entry = info->getEntry(joint);
                    if (entry.targetIndex == -1) {
                        continue;
                    }

                    float32x4_t& trans = retargetBlender->GetResult()[joint].translate._v;
                    float32x4_t dir = normalizeVectorSimd(trans);
                    float32x4_t diff = vsubq_f32(trans, vmulq_n_f32(dir, entry.targetLength));
                    float32x4_t scale = loadVectorSimd(info->getScale());
                    diff = vmulq_f32(diff, scale);
                    float32x4_t result = vaddq_f32(vmulq_n_f32(dir, entry.length), diff);
                    float32x2_t high = vset_lane_f32(0.0f, vget_high_f32(result), 1);
                    trans = vcombine_f32(vget_low_f32(result), high);
                }
            }

            isApply = true;
        }

        if (isApply) {
            mBlender->ApplyTo(mModelObj->GetSkeleton());
        }
    }

    mModelAnimInterp->interpAnim(mModelObj->GetSkeleton());
}

/**
 * Creates the animation interpolator.
 * @param pArcPath Path of the archive holding the interpolation settings.
 */
void AnimPlayerSkl::initInterp(const char* pArcPath) {
    mModelAnimInterp = new ModelAnimInterp(mModelObj->GetSkeleton());
    mModelAnimInterp->initWithArcPath(pArcPath, mInfoTable->getInfoCount());
}

/**
 * Sets the weight of a blended animation.
 * @param index Blend slot index.
 * @param weight Blend weight.
 */
void AnimPlayerSkl::setSklAnimBlendWeight(s32 index, f32 weight) {
    mBlendWeights[index] = weight;
}

/**
 * Gets the weight of a blended animation.
 * @param index Blend slot index.
 * @return Blend weight.
 */
f32 AnimPlayerSkl::getSklAnimBlendWeight(s32 index) const {
    return mBlendWeights[index];
}

/**
 * Gets the number of animations currently playing.
 * @return Number of playing animations.
 */
s32 AnimPlayerSkl::getSklAnimBlendNum() const {
    for (s32 i = 0; i < mAnimObjs.size(); i++) {
        if (mAnimObjs[i] == nullptr) {
            return i;
        }
    }

    return mAnimObjs.size();
}

/**
 * Gets the current frame of an animation.
 * @param index Blend slot index.
 * @return Current frame, or 0 if no animation is playing in the slot.
 */
f32 AnimPlayerSkl::getSklAnimFrame(s32 index) const {
    nn::g3d::SkeletalAnimObj* animObj = mAnimObjs[index];
    if (animObj == nullptr) {
        return 0.0f;
    }

    return animObj->GetFrameCtrl().GetFrame();
}

/**
 * Gets the animation object of a blend slot.
 * @param index Blend slot index.
 * @return Animation object.
 */
nn::g3d::SkeletalAnimObj* AnimPlayerSkl::getAnimObj(s32 index) const {
    return mAnimObjs[index];
}

/**
 * Sets the current frame of an animation.
 * @param index Blend slot index.
 * @param frame New frame.
 * @param isUpdate Whether to recalculate the skeleton right away.
 */
void AnimPlayerSkl::setSklAnimFrame(s32 index, f32 frame, bool isUpdate) {
    _10 = true;
    _11 = true;
    getAnimObj(index)->GetFrameCtrl().SetFrame(frame);

    if (isUpdate) {
        calcSklAnim();
    }
}

/**
 * Gets the animation object of a blend slot.
 * @param index Blend slot index.
 * @return Animation object.
 */
nn::g3d::SkeletalAnimObj* AnimPlayerSkl::getAnimObj(s32 index) {
    return mAnimObjs[index];
}

/**
 * Gets the last frame of an animation.
 * @param index Blend slot index.
 * @return Last frame.
 */
f32 AnimPlayerSkl::getSklAnimFrameMax(s32 index) const {
    return getAnimObj(index)->GetFrameCtrl().GetEndFrame();
}

/**
 * Gets the last frame of an animation.
 * @param pName Animation name.
 * @return Last frame.
 */
f32 AnimPlayerSkl::getSklAnimFrameMax(const char* pName) const {
    return mInfoTable->findAnimInfo(pName)->frameMax;
}

/**
 * Gets the frame rate of an animation.
 * @param index Blend slot index.
 * @return Frame rate, or 0 if no animation is playing in the slot.
 */
f32 AnimPlayerSkl::getSklAnimFrameRate(s32 index) const {
    nn::g3d::SkeletalAnimObj* animObj = mAnimObjs[index];
    if (animObj == nullptr) {
        return 0.0f;
    }

    return animObj->GetFrameCtrl().GetStep();
}

/**
 * Sets the frame rate of an animation.
 * @param index Blend slot index.
 * @param rate New frame rate.
 */
void AnimPlayerSkl::setSklAnimFrameRate(s32 index, f32 rate) {
    _11 = true;
    getAnimObj(index)->GetFrameCtrl().SetStep(rate);
}

/**
 * Checks whether an animation exists.
 * @param pName Animation name.
 * @return Whether the animation exists.
 */
bool AnimPlayerSkl::isSklAnimExist(const char* pName) const {
    return mInfoTable->tryFindAnimInfo(pName) != nullptr;
}

/**
 * Checks whether an animation reached its last frame.
 * @param index Blend slot index.
 * @return Whether the animation ended.
 */
bool AnimPlayerSkl::isSklAnimEnd(s32 index) const {
    nn::g3d::SkeletalAnimObj* animObj = getAnimObj(index);
    return animObj->GetFrameCtrl().GetFrame() >= animObj->GetResource()->GetFrameCount();
}

/**
 * Checks whether an animation plays only once.
 * @param index Blend slot index.
 * @return Whether the animation does not loop.
 */
bool AnimPlayerSkl::isSklAnimOneTime(s32 index) const {
    return !getAnimObj(index)->GetResource()->IsLooped();
}

/**
 * Checks whether an animation plays only once.
 * @param pName Animation name.
 * @return Whether the animation does not loop.
 */
bool AnimPlayerSkl::isSklAnimOneTime(const char* pName) const {
    return !mInfoTable->findAnimInfo(pName)->isLoopAnim;
}

/**
 * Checks whether an animation is playing in a blend slot.
 * @param index Blend slot index.
 * @return Whether an animation is playing.
 */
bool AnimPlayerSkl::isSklAnimPlaying(s32 index) const {
    return mAnimObjs[index] != nullptr;
}

/**
 * Gets the name of the animation playing in a blend slot.
 * @param index Blend slot index.
 * @return Animation name, or nullptr if no animation is playing.
 */
const char* AnimPlayerSkl::getPlayingSklAnimName(s32 index) const {
    nn::g3d::SkeletalAnimObj* animObj = mAnimObjs[index];
    if (animObj == nullptr) {
        return nullptr;
    }

    return animObj->GetResource()->GetName();
}

/**
 * Advances the animations by one frame.
 */
void AnimPlayerSkl::update() {
    if (mModelAnimInterp != nullptr) {
        mModelAnimInterp->update();
    }

    if (!_11) {
        return;
    }

    if (!_10) {
        for (u32 i = 0; i < mAnimObjs.size(); i++) {
            if (mAnimObjs[i] != nullptr) {
                mAnimObjs[i]->GetFrameCtrl().UpdateFrame();
            }
        }
    }

    for (u32 i = 0; i < mPartialAnimInfos.size(); i++) {
        if (mPartialAnimInfos[i].isAttached && !mPartialAnimInfos[i].isFrameSet) {
            mPartialAnimInfos[i].animObj->GetFrameCtrl().UpdateFrame();
        }
    }
}

/**
 * Cancels the interpolation and recalculates the skeleton.
 */
void AnimPlayerSkl::reset() {
    mModelAnimInterp->prepareNoInterp();
    calcSklAnim();
}

/**
 * Checks whether the animation needs to be updated next frame.
 * @return Whether the animation needs to be updated.
 */
bool AnimPlayerSkl::calcNeedUpdateAnimNext() {
    if (!_11) {
        return false;
    }

    if (isBlend() || isAttachedPartialAnim()) {
        return true;
    }

    if (isSklAnimPlaying(0)) {
        if (!isSklAnimOneTime(0)) {
            return true;
        }

        if (!isSklAnimEnd(0)) {
            return true;
        }
    }

    _11 = false;
    return true;
}

/**
 * Starts an interpolation with a given duration.
 * @param interpFrame Number of interpolation frames.
 */
void AnimPlayerSkl::prepareAnimInterpDirect(s32 interpFrame) {
    mModelAnimInterp->prepareAnimInterp(mModelObj->GetSkeleton(), interpFrame, mAnimObjs);
}

/**
 * Creates the partial animation slots and joint groups.
 * @param slotNum Number of partial animation slots.
 * @param jointGroupNum Number of joint groups.
 * @param jointNum Maximum number of joints per group.
 */
void AnimPlayerSkl::initPartialAnim(s32 slotNum, s32 jointGroupNum, s32 jointNum) {
    nn::g3d::SkeletalAnimObj::InitializeArgument arg;
    arg.SetMaxBoneCount(mModelObj->GetSkeleton()->GetRes()->GetBoneCount());
    for (s32 i = 0; i < mInfoTable->getInfoCount(); i++) {
        arg.Reserve(
            static_cast<const nn::g3d::ResSkeletalAnim*>(mInfoTable->getResInfo(i).resAnim));
    }

    mPartialAnimInfos.tryAllocBuffer(slotNum, nullptr);
    for (s32 i = 0; i < mPartialAnimInfos.size(); i++) {
        mPartialAnimInfos[i].isAttached = false;
        mPartialAnimInfos[i].isFrameSet = false;

        nn::g3d::SkeletalAnimObj* animObj = new nn::g3d::SkeletalAnimObj();
        arg.CalculateMemorySize();
        size_t size = arg.GetWorkMemorySize();
        void* buffer = new (16) u8[size];
        memset(buffer, 0, size);
        animObj->Initialize(arg, buffer, size);

        mPartialAnimInfos[i].animObj = animObj;
        mPartialAnimInfos[i].retargettingInfo = nullptr;
        mPartialAnimInfos[i].joints = nullptr;
    }

    mJointIndexArrays = new JointIndexArray[jointGroupNum];
    for (s32 i = 0; i < jointGroupNum; i++) {
        mJointIndexArrays[i].allocBuffer(jointNum, nullptr);
    }

    mJointIndexArrayNum = jointGroupNum;
}

/**
 * Gets the number of partial animation slots.
 * @return Number of slots, or 0 if partial animations were not initialized.
 */
s32 AnimPlayerSkl::getPartialAnimSlotNum() const {
    if (!mPartialAnimInfos.isBufferReady()) {
        return 0;
    }

    return mPartialAnimInfos.size();
}

/**
 * Counts a joint and all of its descendants.
 * @param pJointName Name of the root joint.
 * @return Number of joints, or 0 if the joint does not exist.
 */
s32 AnimPlayerSkl::getJoitsAmountFromJoint(const char* pJointName) const {
    const nn::g3d::SkeletonObj* skeleton = mModelObj->GetSkeleton();
    s32 jointIndex = skeleton->FindBoneIndex(pJointName);
    if (jointIndex == -1) {
        return 0;
    }

    return countJointsRecursive(skeleton, jointIndex);
}

/**
 * Adds the joints between two joints (inclusive) to a joint group.
 * @param groupIndex Joint group index.
 * @param pRootJointName Name of the topmost joint.
 * @param pEndJointName Name of the bottommost joint.
 */
void AnimPlayerSkl::addPartialAnimJoint(s32 groupIndex, const char* pRootJointName,
                                        const char* pEndJointName) {
    s32 rootIndex = mModelObj->GetSkeleton()->FindBoneIndex(pRootJointName);
    s32 jointIndex = mModelObj->GetSkeleton()->FindBoneIndex(pEndJointName);

    while (jointIndex != rootIndex) {
        mJointIndexArrays[groupIndex].emplaceBack(jointIndex);
        jointIndex = mModelObj->GetSkeleton()->GetBone(jointIndex)->GetParentIndex();
    }

    mJointIndexArrays[groupIndex].emplaceBack(rootIndex);
}

/**
 * Adds a joint and all of its descendants to a joint group.
 * @param groupIndex Joint group index.
 * @param pJointName Name of the root joint.
 */
void AnimPlayerSkl::addPartialAnimJointRecursive(s32 groupIndex, const char* pJointName) {
    const nn::g3d::SkeletonObj* skeleton = mModelObj->GetSkeleton();
    s32 jointIndex = skeleton->FindBoneIndex(pJointName);
    addJointRecursive(&mJointIndexArrays[groupIndex], skeleton, jointIndex);
}

/**
 * Starts a partial animation.
 * @param pName Animation name.
 * @param slot Partial animation slot.
 * @param groupIndex Joint group the animation is applied to.
 * @param pRetargettingInfo Retargetting info, or nullptr.
 */
void AnimPlayerSkl::startPartialAnim(const char* pName, s32 slot, s32 groupIndex,
                                     const SklAnimRetargettingInfo* pRetargettingInfo) {
    const AnimResInfo* info = mInfoTable->findAnimInfo(pName);

    if (info != nullptr) {
        mPartialAnimInfos[slot].animObj->SetResource(
            static_cast<const nn::g3d::ResSkeletalAnim*>(info->resAnim));
        mPartialAnimInfos[slot].animObj->Bind(mModelObj->GetSkeleton()->GetRes());
        mPartialAnimInfos[slot].retargettingInfo = pRetargettingInfo;
        mPartialAnimInfos[slot].isAttached = true;
        mPartialAnimInfos[slot].isFrameSet = true;
    } else {
        mPartialAnimInfos[slot].isAttached = false;
    }

    mPartialAnimInfos[slot].joints = &mJointIndexArrays[groupIndex];
    mPartialAnimInfos[slot].animObj->GetFrameCtrl().SetFrame(0.0f);
    mPartialAnimInfos[slot].animObj->GetFrameCtrl().SetStep(1.0f);
    _11 = true;
}

/**
 * Stops a partial animation.
 * @param slot Partial animation slot.
 */
void AnimPlayerSkl::clearPartialAnim(s32 slot) {
    PartialAnimInfo& partial = mPartialAnimInfos[slot];
    partial.isAttached = false;
    partial.retargettingInfo = nullptr;
    partial.joints = nullptr;
}

/**
 * Gets a partial animation slot.
 * @param slot Partial animation slot.
 * @return Partial animation info.
 */
AnimPlayerSkl::PartialAnimInfo* AnimPlayerSkl::getPartialAnimInfo(s32 slot) {
    return &mPartialAnimInfos[slot];
}

/**
 * Checks whether a partial animation reached its last frame.
 * @param slot Partial animation slot.
 * @return Whether the animation ended.
 */
bool AnimPlayerSkl::isPartialAnimEnd(s32 slot) const {
    nn::g3d::SkeletalAnimObj* animObj = getPartialAnimObj(slot);
    return animObj->GetFrameCtrl().GetFrame() >= animObj->GetResource()->GetFrameCount();
}

/**
 * Gets the animation object of a partial animation slot.
 * @param slot Partial animation slot.
 * @return Animation object.
 */
nn::g3d::SkeletalAnimObj* AnimPlayerSkl::getPartialAnimObj(s32 slot) const {
    return getPartialAnimInfo(slot)->animObj;
}

/**
 * Checks whether a partial animation plays only once.
 * @param slot Partial animation slot.
 * @return Whether the animation does not loop.
 */
bool AnimPlayerSkl::isPartialAnimOneTime(s32 slot) const {
    return getPartialAnimObj(slot)->GetFrameCtrl().GetPlayPolicy() ==
           nn::g3d::AnimFrameCtrl::PlayOneTime;
}

/**
 * Checks whether a partial animation is playing.
 * @param slot Partial animation slot.
 * @return Whether the animation is attached.
 */
bool AnimPlayerSkl::isPartialAnimAttached(s32 slot) const {
    return getPartialAnimInfo(slot)->isAttached;
}

/**
 * Gets a partial animation slot.
 * @param slot Partial animation slot.
 * @return Partial animation info.
 */
const AnimPlayerSkl::PartialAnimInfo* AnimPlayerSkl::getPartialAnimInfo(s32 slot) const {
    return &mPartialAnimInfos[slot];
}

/**
 * Gets the name of a partial animation.
 * @param slot Partial animation slot.
 * @return Animation name.
 */
const char* AnimPlayerSkl::getPlayingPartialSklAnimName(s32 slot) const {
    return getPartialAnimObj(slot)->GetResource()->GetName();
}

/**
 * Gets the current frame of a partial animation.
 * @param slot Partial animation slot.
 * @return Current frame.
 */
f32 AnimPlayerSkl::getPartialAnimFrame(s32 slot) const {
    return getPartialAnimObj(slot)->GetFrameCtrl().GetFrame();
}

/**
 * Sets the current frame of a partial animation.
 * @param slot Partial animation slot.
 * @param frame New frame.
 */
void AnimPlayerSkl::setPartialAnimFrame(s32 slot, f32 frame) {
    mPartialAnimInfos[slot].isFrameSet = true;
    mPartialAnimInfos[slot].animObj->GetFrameCtrl().SetFrame(frame);
}

/**
 * Gets the animation object of a partial animation slot.
 * @param slot Partial animation slot.
 * @return Animation object.
 */
nn::g3d::SkeletalAnimObj* AnimPlayerSkl::getPartialAnimObj(s32 slot) {
    return getPartialAnimInfo(slot)->animObj;
}

/**
 * Gets the frame rate of a partial animation.
 * @param slot Partial animation slot.
 * @return Frame rate.
 */
f32 AnimPlayerSkl::getPartialAnimFrameRate(s32 slot) const {
    return getPartialAnimObj(slot)->GetFrameCtrl().GetStep();
}

/**
 * Sets the frame rate of a partial animation.
 * @param slot Partial animation slot.
 * @param rate New frame rate.
 */
void AnimPlayerSkl::setPartialAnimFrameRate(s32 slot, f32 rate) {
    getPartialAnimObj(slot)->GetFrameCtrl().SetStep(rate);
}

/**
 * Constructs a skeletal animation player.
 * @param pInfo Animation player init info.
 * @param blendNum Maximum number of blended animations.
 */
AnimPlayerSkl::AnimPlayerSkl(const AnimPlayerInitInfo* pInfo, s32 blendNum)
    : mModelObj(pInfo->mModelObj) {
    mBlender = new nn::g3d::SkeletalAnimBlender();
    mInfoTable = pInfo->mInitResourceDataAnim->getSklAnimInfoTable();

    mAnimObjs.tryAllocBuffer(blendNum, nullptr);
    for (s32 i = 0; i < mAnimObjs.size(); i++) {
        mAnimObjs[i] = nullptr;
    }

    {
        nn::g3d::SkeletalAnimObj::InitializeArgument arg;
        arg.SetMaxBoneCount(mModelObj->GetSkeleton()->GetRes()->GetBoneCount());
        for (s32 i = 0; i < mInfoTable->getInfoCount(); i++) {
            arg.Reserve(
                static_cast<const nn::g3d::ResSkeletalAnim*>(mInfoTable->getResInfo(i).resAnim));
        }

        mAnimObjStack.tryAllocBuffer(blendNum, nullptr);
        for (s32 i = 0; i < blendNum; i++) {
            nn::g3d::SkeletalAnimObj* animObj = new nn::g3d::SkeletalAnimObj();
            arg.CalculateMemorySize();
            size_t size = arg.GetWorkMemorySize();
            void* buffer = new (16) u8[size];
            memset(buffer, 0, size);
            animObj->Initialize(arg, buffer, size);
            mAnimObjStack[i] = animObj;
        }
    }

    nn::g3d::SkeletalAnimBlender::InitializeArgument blenderArg;
    blenderArg.SetMaxBoneCount(mModelObj->GetResource()->GetSkeleton()->GetBoneCount());
    blenderArg.CalculateMemorySize();
    u32 blenderSize = blenderArg.GetWorkMemorySize();
    void* blenderBuffer = new (16) u8[blenderSize];
    mBlender->Initialize(blenderArg, blenderBuffer, blenderSize);

    mBlendWeights.tryAllocBuffer(blendNum, nullptr);
    f32* weights = mBlendWeights.getBufferPtr();
    for (s32 i = 0; i != mBlendWeights.size(); i++) {
        weights[i] = 0.0f;
    }
}

}  // namespace al

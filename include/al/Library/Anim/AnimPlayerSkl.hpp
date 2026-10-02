#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <container/seadObjArray.h>

#include "Project/Animation/AnimPlayerBase.hpp"

namespace nn::g3d {
class ModelObj;
class SkeletalAnimBlender;
class SkeletalAnimObj;
}  // namespace nn::g3d

namespace al {
struct AnimPlayerInitInfo;
class ModelAnimInterp;
class SklAnimRetargettingInfo;

/// Plays skeletal animations on a model, with blending, interpolation and partial animations.
class AnimPlayerSkl : public AnimPlayerBase {
public:
    using JointIndexArray = sead::ObjArray<s32>;

    /// A skeletal animation that is only applied to a subset of the joints.
    struct PartialAnimInfo {
        bool isAttached;
        bool isFrameSet;
        nn::g3d::SkeletalAnimObj* animObj;
        const SklAnimRetargettingInfo* retargettingInfo;
        JointIndexArray* joints;
    };

    static_assert(sizeof(PartialAnimInfo) == 0x20);

    static AnimPlayerSkl* tryCreate(const AnimPlayerInitInfo* pInfo, s32 blendNum);

    AnimPlayerSkl(const AnimPlayerInitInfo* pInfo, s32 blendNum);

    void updateLast() override;
    bool calcNeedUpdateAnimNext() override;

    void startSklAnim(const char* pInterpAnimName, const char* pName0, const char* pName1,
                      const char* pName2, const char* pName3, const char* pName4,
                      const char* pName5);
    void clearSklAnimBlend();
    void setSklAnim(const char* pName, s32 index);
    void calcSklAnim();
    void initInterp(const char* pArcPath);
    void setSklAnimBlendWeight(s32 index, f32 weight);
    f32 getSklAnimBlendWeight(s32 index) const;
    s32 getSklAnimBlendNum() const;
    f32 getSklAnimFrame(s32 index) const;
    nn::g3d::SkeletalAnimObj* getAnimObj(s32 index) const;
    void setSklAnimFrame(s32 index, f32 frame, bool isUpdate);
    nn::g3d::SkeletalAnimObj* getAnimObj(s32 index);
    f32 getSklAnimFrameMax(s32 index) const;
    f32 getSklAnimFrameMax(const char* pName) const;
    f32 getSklAnimFrameRate(s32 index) const;
    void setSklAnimFrameRate(s32 index, f32 rate);
    bool isSklAnimExist(const char* pName) const;
    bool isSklAnimEnd(s32 index) const;
    bool isSklAnimOneTime(s32 index) const;
    bool isSklAnimOneTime(const char* pName) const;
    bool isSklAnimPlaying(s32 index) const;
    const char* getPlayingSklAnimName(s32 index) const;
    void update();
    void reset();
    void prepareAnimInterpDirect(s32 interpFrame);
    void initPartialAnim(s32 slotNum, s32 jointGroupNum, s32 jointNum);
    s32 getPartialAnimSlotNum() const;
    s32 getJoitsAmountFromJoint(const char* pJointName) const;
    void addPartialAnimJoint(s32 groupIndex, const char* pRootJointName,
                             const char* pEndJointName);
    void addPartialAnimJointRecursive(s32 groupIndex, const char* pJointName);
    void startPartialAnim(const char* pName, s32 slot, s32 groupIndex,
                          const SklAnimRetargettingInfo* pRetargettingInfo);
    void clearPartialAnim(s32 slot);
    PartialAnimInfo* getPartialAnimInfo(s32 slot);
    bool isPartialAnimEnd(s32 slot) const;
    nn::g3d::SkeletalAnimObj* getPartialAnimObj(s32 slot) const;
    bool isPartialAnimOneTime(s32 slot) const;
    bool isPartialAnimAttached(s32 slot) const;
    const PartialAnimInfo* getPartialAnimInfo(s32 slot) const;
    const char* getPlayingPartialSklAnimName(s32 slot) const;
    f32 getPartialAnimFrame(s32 slot) const;
    void setPartialAnimFrame(s32 slot, f32 frame);
    nn::g3d::SkeletalAnimObj* getPartialAnimObj(s32 slot);
    f32 getPartialAnimFrameRate(s32 slot) const;
    void setPartialAnimFrameRate(s32 slot, f32 rate);

    bool isBlend() const {
        for (u32 i = 1; i < mAnimObjs.size(); i++) {
            if (mAnimObjs[i] != nullptr) {
                return true;
            }
        }

        return false;
    }

    bool isAttachedPartialAnim() const {
        for (s32 i = 0; i < mPartialAnimInfos.size(); i++) {
            if (mPartialAnimInfos[i].isAttached) {
                return true;
            }
        }

        return false;
    }

    nn::g3d::ModelObj* mModelObj = nullptr;
    sead::Buffer<nn::g3d::SkeletalAnimObj*> mAnimObjs;
    sead::Buffer<nn::g3d::SkeletalAnimObj*> mAnimObjStack;
    ModelAnimInterp* mModelAnimInterp = nullptr;
    nn::g3d::SkeletalAnimBlender* mBlender = nullptr;
    sead::Buffer<f32> mBlendWeights;
    sead::Buffer<PartialAnimInfo> mPartialAnimInfos;
    JointIndexArray* mJointIndexArrays = nullptr;
    s32 mJointIndexArrayNum = 0;
    const SklAnimRetargettingInfo* mRetargettingInfo = nullptr;
    bool mIsRetargettingValid = false;
    bool mIsSkipInterp = true;
};

static_assert(sizeof(AnimPlayerSkl) == 0x90);

}  // namespace al

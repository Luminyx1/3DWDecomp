#pragma once
#include <nn/types.h>
#include <nn/g3d/g3d_ResAnimCurve.h>

namespace nn::g3d {
class ResModel;
class ModelObj;
class BindResult;
class AnimFrameCtrl {
public:
    using PlayPolicy = float (*)(float frame, float start, float end, void* user);
    AnimFrameCtrl()
        : mFrame(0.0f), mStart(0.0f), mEnd(0.0f), mStep(1.0f), mPolicy(PlayOneTime),
          mUser(nullptr) {}
    static const float InvalidFrame;
    float GetFrame() const { return mFrame; }
    // frame is passed through the play policy before being stored.
    void SetFrame(float frame) { mFrame = mPolicy(frame, mStart, mEnd, mUser); }
    void UpdateFrame() { mFrame = mPolicy(mFrame + mStep, mStart, mEnd, mUser); }
    float GetStartFrame() const { return mStart; }
    float GetEndFrame() const { return mEnd; }
    float GetStep() const { return mStep; }
    void SetStep(float step) { mStep = step; }
    PlayPolicy GetPlayPolicy() const { return mPolicy; }
    void Initialize(float start, float end, PlayPolicy policy);
    static float PlayOneTime(float frame, float start, float end, void* user);
    static float PlayLoop(float frame, float start, float end, void* user);
private:
    float mFrame, mStart, mEnd, mStep;
    PlayPolicy mPolicy;
    void* mUser;
};
class AnimBindTable {
public:
    AnimBindTable() : mEntries(nullptr), mFlags(0), mCapacity(0), mAnimCount(0), mTargetCount(0) {}
    bool IsBound() const { return mFlags & 1; }
    void Initialize(u32* buffer, int capacity);
    void ClearAll(int targetCount);
    void BindAll(const u16* indices);
    /**
     * @brief Replace calculation and application flags for the animation bound to a target.
     * @param targetIndex Target index within the initialized binding table.
     * @param flags Encoded binding flags in bits 30 and 31; all other bits must be zero.
     */
    void SetFlagsForTarget(ptrdiff_t targetIndex, u32 flags) {
        u32 index = (mEntries[targetIndex] >> 15) & 0x7fff;
        if (index != 0x7fff) {
            mEntries[index] &= 0x3fffffff;
            mEntries[index] |= flags;
        }
    }
private:
    friend class ModelAnimObj;
    friend class BoneVisibilityAnimObj;
    friend class MaterialAnimObj;
    friend class SkeletalAnimObj;
    u32* mEntries;
    u16 mFlags, mCapacity, mAnimCount, mTargetCount;
};
class AnimContext {
public:
    AnimContext() : mCache(nullptr), mCount(0), mCurveCount(0), mLastFrame(0.0f) {}
    // count is the number of curves whose cached intervals should be invalidated.
    void SetCurveCount(int count) { mCurveCount = count; Reset(); }
    bool IsCacheValid() const { return mCurveCount > 0 && mCurveCount <= mCount; }
    void Reset() {
        mLastFrame = AnimFrameCtrl::InvalidFrame;
        if (IsCacheValid()) for (int i = 0; i < mCurveCount; ++i) mCache[i].start = __builtin_inff();
    }
    void Initialize(AnimFrameCache* cache, int count);
private:
    friend class BoneVisibilityAnimObj;
    friend class MaterialAnimObj;
    friend class SkeletalAnimObj;
    AnimFrameCache* mCache;
    int mCount;
    int mCurveCount;
    float mLastFrame;
    u32 _14;
};
class AnimObj {
public:
    AnimObj() : mResult(nullptr), mWorkMemory(nullptr) { mFrameCtrlPointer = &mFrameCtrl; }
    /** @brief Destroy the animation base without releasing caller-owned storage. */
    virtual ~AnimObj() {}
    AnimFrameCtrl& GetFrameCtrl() { return *mFrameCtrlPointer; }
    const AnimFrameCtrl& GetFrameCtrl() const { return *mFrameCtrlPointer; }
    virtual void ClearResult() = 0;
    virtual void Calculate() = 0;
    enum BindFlag { BindFlag_None, BindFlag_SkipCalculate, BindFlag_SkipApply, BindFlag_Disable };
    void ResetFrameCtrl(int frameCount, bool loop);
protected:
    AnimFrameCtrl* mFrameCtrlPointer;
    AnimFrameCtrl mFrameCtrl;
    AnimContext mContext;
    void* mResult;
    void* mWorkMemory;
};
class ModelAnimObj : public AnimObj {
public:
    virtual BindResult Bind(const ResModel* model) = 0;
    virtual BindResult Bind(const ModelObj* model) = 0;
    virtual void BindFast(const ResModel* model) = 0;
    virtual void ApplyTo(ModelObj* model) const = 0;
    void SetBindFlagImpl(int targetIndex, BindFlag flag);
    BindFlag GetBindFlagImpl(int targetIndex) const;
    bool IsBound() const { return mBindTable.IsBound(); }
protected:
    AnimBindTable mBindTable;
};
static_assert(sizeof(AnimFrameCtrl) == 0x20, "Animation frame controller size");
static_assert(sizeof(AnimBindTable) == 0x10, "Animation binding table size");
}

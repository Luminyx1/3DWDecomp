#pragma once
#include <nn/types.h>
#include <nn/g3d/g3d_ResAnimCurve.h>

namespace nn::g3d {
class ResModel;
class ModelObj;
class BindResult;
class AnimFrameCtrl {
public:
    static const float InvalidFrame;
    float GetFrame() const { return mFrame; }
    using PlayPolicy = float (*)(float frame, float start, float end, void* user);
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
    void Initialize(u32* buffer, int capacity);
    void ClearAll(int targetCount);
    void BindAll(const u16* indices);
private:
    friend class ModelAnimObj;
    friend class BoneVisibilityAnimObj;
    u32* mEntries;
    u16 mFlags, mCapacity, mAnimCount, mTargetCount;
};
class AnimContext {
public:
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
    AnimFrameCache* mCache;
    int mCount;
    int mCurveCount;
    float mLastFrame;
    u32 _14;
};
class AnimObj {
public:
    virtual ~AnimObj() {}
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
protected:
    AnimBindTable mBindTable;
};
static_assert(sizeof(AnimFrameCtrl) == 0x20, "Animation frame controller size");
static_assert(sizeof(AnimBindTable) == 0x10, "Animation binding table size");
}

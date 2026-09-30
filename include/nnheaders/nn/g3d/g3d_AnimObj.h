#pragma once
#include <nn/types.h>

namespace nn::g3d {
class AnimFrameCtrl {
public:
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
    u32* mEntries;
    u16 mFlags, mCapacity, mAnimCount, mTargetCount;
};
struct AnimFrameCache;
class AnimContext {
public:
    void Initialize(AnimFrameCache* cache, int count);
private:
    AnimFrameCache* mCache;
    int mCount;
    int mFlags;
};
class AnimObj {
public:
    enum BindFlag { BindFlag_None, BindFlag_SkipCalculate, BindFlag_SkipApply, BindFlag_Disable };
    void ResetFrameCtrl(int frameCount, bool loop);
private:
    u8 _0[0x10];
    AnimFrameCtrl mFrameCtrl;
    u8 _30[0x58 - 0x30];
};
class ModelAnimObj : public AnimObj {
public:
    void SetBindFlagImpl(int targetIndex, BindFlag flag);
    BindFlag GetBindFlagImpl(int targetIndex) const;
private:
    AnimBindTable mBindTable;
};
static_assert(sizeof(AnimFrameCtrl) == 0x20, "Animation frame controller size");
static_assert(sizeof(AnimBindTable) == 0x10, "Animation binding table size");
}

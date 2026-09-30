#include <nn/g3d/g3d_AnimObj.h>

namespace nn::g3d {
// start and end bound playback; policy selects the handling of out-of-range frames.
void AnimFrameCtrl::Initialize(float start, float end, PlayPolicy policy) {
    mFrame = 0;
    mStart = start;
    mEnd = end;
    mStep = 1;
    mPolicy = policy;
    mUser = nullptr;
}
// frame is the requested position, start/end bound playback, and user is unused by this policy.
float AnimFrameCtrl::PlayOneTime(float frame, float start, float end, void* user) {
    float result = frame < start ? start : frame;
    if (frame >= end) result = end;
    return result;
}
// frame is wrapped into start/end; user is unused by the built-in looping policy.
float AnimFrameCtrl::PlayLoop(float frame, float start, float end, void* user) {
    float distance, direction, origin;
    if (frame >= end) {
        distance = frame - end;
        direction = 1;
        origin = start;
    } else if (frame < start) {
        distance = start - frame;
        direction = -1;
        origin = end;
    } else return frame;
    float length = end - start;
    if (length == 0) return start;
    return origin + direction * (distance - length * static_cast<int>(distance / length));
}
// buffer supplies packed forward/reverse bindings; capacity is its entry count.
void AnimBindTable::Initialize(u32* buffer, int capacity) {
    mEntries = buffer;
    mFlags = 0;
    mCapacity = capacity;
    mAnimCount = 0;
    mTargetCount = 0;
}
// targetCount sets the target count; clear enough entries for both sides of the binding.
void AnimBindTable::ClearAll(int targetCount) {
    u16 count = mAnimCount < static_cast<u16>(targetCount) ? static_cast<u16>(targetCount) : mAnimCount;
    mTargetCount = targetCount;
    for (int i = 0; i < count; ++i) mEntries[i] = 0xffffffff;
}
// indices maps each animation entry to a target; values above 0x7ffe are unbound.
void AnimBindTable::BindAll(const u16* indices) {
    for (int i = 0; i < mAnimCount; ++i) {
        u16 target = indices[i];
        if (target <= 0x7ffe) {
            mEntries[i] &= 0x3fff8000;
            mEntries[i] |= target & 0x7fff;
            mEntries[target] &= 0xc0007fff;
            mEntries[target] |= (i << 15) & 0x3fff8000;
        }
    }
}
// cache supplies per-curve frame caches; count is ignored when cache is null.
void AnimContext::Initialize(AnimFrameCache* cache, int count) {
    mCache = cache;
    mCount = (cache != nullptr) ? count : 0;
    mCurveCount = 0;
}
// frameCount sets the end frame; loop chooses wrapping instead of clamping playback.
void AnimObj::ResetFrameCtrl(int frameCount, bool loop) {
    mFrameCtrl.Initialize(0, frameCount, loop ? AnimFrameCtrl::PlayLoop : AnimFrameCtrl::PlayOneTime);
}
// targetIndex selects a model target; flag controls its bound animation's calculation/application.
void ModelAnimObj::SetBindFlagImpl(int targetIndex, BindFlag flag) {
    u32 index = (mBindTable.mEntries[targetIndex] >> 15) & 0x7fff;
    if (index != 0x7fff) {
        mBindTable.mEntries[index] &= 0x3fffffff;
        mBindTable.mEntries[index] |= static_cast<u32>(flag) << 30;
    }
}
// targetIndex selects a model target; unbound targets return BindFlag_Disable.
AnimObj::BindFlag ModelAnimObj::GetBindFlagImpl(int targetIndex) const {
    u32 index = (mBindTable.mEntries[targetIndex] >> 15) & 0x7fff;
    if (index == 0x7fff) return BindFlag_Disable;
    return static_cast<BindFlag>(mBindTable.mEntries[index] >> 30);
}
}

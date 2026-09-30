#pragma once
#include <nn/types.h>

namespace nn::g3d {
struct AnimFrameCache {
    float start;
    float end;
    int keyIndex;
    AnimFrameCache() : start(__builtin_inff()) {}
    // frame is the sample time within this cached key interval.
    float GetInterpolationWeight(float frame) const { return (frame - start) * (1.0f / (end - start)); }
};

class ResAnimCurve {
  public:
    // frame is the sample time; cache retains the current key interval between samples.
    float EvaluateFloat(float frame, AnimFrameCache* cache) const;
    int EvaluateInt(float frame, AnimFrameCache* cache) const;
    size_t CalculateBakedFloatSize() const;
    size_t CalculateBakedIntSize() const;
    // buffer supplies size bytes of writable storage for baked samples.
    void BakeFloat(void* buffer, size_t size);
    void BakeInt(void* buffer, size_t size);
    void ResetFloat();
    void ResetInt();
    void Reset() {
        if (flags & 0x40)
            ResetInt();
        else
            ResetFloat();
    }

    // cache receives the interval containing frame; T is the stored frame/key representation.
    void UpdateFrameCache(AnimFrameCache* cache, float frame) const;
    template <class T> void FindFrame(AnimFrameCache* cache, float frame) const;
    template <class T> float EvaluateCubic(float frame, AnimFrameCache* cache) const;
    template <class T> float EvaluateLinear(float frame, AnimFrameCache* cache) const;
    template <class T> float EvaluateBakedFloat(float frame, AnimFrameCache* cache) const;
    template <class T> int EvaluateStepInt(float frame, AnimFrameCache* cache) const;
    template <class T> int EvaluateBakedInt(float frame, AnimFrameCache* cache) const;
    int EvaluateStepBool(float frame, AnimFrameCache* cache) const;
    int EvaluateBakedBool(float frame, AnimFrameCache* cache) const;
    // buffer receives count samples starting on the integer-frame grid at firstFrame.
    template <class T> void BakeImpl(void* buffer, float firstFrame, int count);
    // T selects the stored frame/key representation without changing the resource's pointer layout.
    template <class T> const T* GetFrameArray() const { return static_cast<const T*>(frames); }
    template <class T> const T* GetKeyArray() const { return static_cast<const T*>(keys); }

  private:
    inline int GetBakedFloatIntervalCount() const;
    inline int GetBakedIntIntervalCount() const;

  public:
    struct Impl;

    void* frames;
    void* keys;
    u16 flags;
    u16 keyCount;
    u32 targetOffset;
    float startFrame;
    float endFrame;
    float scale;
    union {
        float offsetFloat;
        int offsetInt;
    };
    union {
        float deltaFloat;
        int deltaInt;
    };
    u8 _2c[4];
};
static_assert(sizeof(ResAnimCurve) == 0x30, "ResAnimCurve resource layout");
} // namespace nn::g3d

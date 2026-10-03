#pragma once
#include <nn/types.h>

namespace nn::g3d {
struct AnimFrameCache {
    float start;
    float end;
    int keyIndex;
    /** @brief Mark the frame cache invalid until its first curve lookup. */
    AnimFrameCache() : start(__builtin_inff()) {}
    /**
     * @brief Calculate a sample's interpolation weight within the cached interval.
     * @param frame Sample time within this nonzero interval.
     * @return Fractional position from start toward end.
     */
    float GetInterpolationWeight(float frame) const { return (frame - start) * (1.0f / (end - start)); }
};

class ResAnimCurve {
  public:
    float EvaluateFloat(float frame, AnimFrameCache* cache) const;
    int EvaluateInt(float frame, AnimFrameCache* cache) const;
    size_t CalculateBakedFloatSize() const;
    size_t CalculateBakedIntSize() const;
    void BakeFloat(void* buffer, size_t size);
    void BakeInt(void* buffer, size_t size);
    void ResetFloat();
    void ResetInt();
    /** @brief Restore original keys using the curve's floating-point or discrete format. */
    void Reset() {
        if (flags & 0x40) {
            ResetInt();
        } else {
            ResetFloat();
        }
    }

    void UpdateFrameCache(AnimFrameCache* cache, float frame) const;
    template <class T> void FindFrame(AnimFrameCache* cache, float frame) const;
    template <class T> float EvaluateCubic(float frame, AnimFrameCache* cache) const;
    template <class T> float EvaluateLinear(float frame, AnimFrameCache* cache) const;
    template <class T> float EvaluateBakedFloat(float frame, AnimFrameCache* cache) const;
    template <class T> int EvaluateStepInt(float frame, AnimFrameCache* cache) const;
    template <class T> int EvaluateBakedInt(float frame, AnimFrameCache* cache) const;
    int EvaluateStepBool(float frame, AnimFrameCache* cache) const;
    int EvaluateBakedBool(float frame, AnimFrameCache* cache) const;
    template <class T> void BakeImpl(void* buffer, float firstFrame, int count);
    /**
     * @brief Access the packed frame positions.
     * @tparam T Stored element representation, which must agree with the curve flags.
     * @return Typed frame array owned by the resource.
     */
    template <class T> const T* GetFrameArray() const { return static_cast<const T*>(frames); }
    /**
     * @brief Access the packed key values or coefficients.
     * @tparam T Stored element representation, which must agree with the curve flags.
     * @return Typed key array owned by the resource or baking buffer.
     */
    template <class T> const T* GetKeyArray() const { return static_cast<const T*>(keys); }

  private:
    /**
     * @brief Normalize a sample time to the curve's frame interval.
     * @param frame Requested sample time; the curve must have a nonzero interval.
     * @return Sample position relative to the start, scaled by the reciprocal interval.
     */
    inline float GetNormalizedFrame(float frame) const {
        float inverse = 1.0f / (endFrame - startFrame);
        return (frame - startFrame) * inverse;
    }
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

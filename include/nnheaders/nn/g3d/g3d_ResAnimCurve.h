#pragma once
#include <nn/types.h>

namespace nn::g3d {
struct AnimFrameCache {
    float start;
    float end;
    int keyIndex;
    AnimFrameCache() : start(__builtin_inff()) {}
};

class ResAnimCurve {
public:
    // frame is the sample time; cache retains the current key interval between samples.
    float EvaluateFloat(float frame, AnimFrameCache* cache) const;
    int EvaluateInt(float frame, AnimFrameCache* cache) const;
    size_t CalculateBakedFloatSize() const;
    // buffer supplies size bytes of writable storage for baked samples.
    bool BakeFloat(void* buffer, size_t size);
    void ResetFloat();
    void ResetInt();
    void Reset() { if (flags & 0x40) ResetInt(); else ResetFloat(); }

    void* frames;
    void* keys;
    u16 flags;
    u16 keyCount;
    u32 targetOffset;
    u8 _18[0x18];
};
}

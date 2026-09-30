#include <nn/g3d/g3d_ResAnimCurve.h>
#include <cmath>
#include <cstddef>
#include <type_traits>

namespace nn::g3d {
// Floating frames remain continuous; quantized frames use their integer comparison domain.
template <class T>
using FrameValue = typename std::conditional<
    std::is_same<T, float>::value, float,
    typename std::conditional<std::is_same<T, s16>::value, int, unsigned>::type>::type;

inline int ResAnimCurve::GetBakedFloatIntervalCount() const {
    return int(std::floor(endFrame) + 1.0f) - int(std::floor(startFrame));
}

inline int ResAnimCurve::GetBakedIntIntervalCount() const {
    return int(std::floor(endFrame)) - int(std::floor(startFrame));
}

struct ResAnimCurve::Impl {
    using FrameFinder = void (ResAnimCurve::*)(AnimFrameCache*, float) const;
    using FloatEvaluator = float (ResAnimCurve::*)(float, AnimFrameCache*) const;
    using IntEvaluator = int (ResAnimCurve::*)(float, AnimFrameCache*) const;
    static const FrameFinder s_pFuncFindFrame[3];
    static FloatEvaluator s_pFuncEvaluateFloat[3][3];
    static IntEvaluator s_pFuncEvaluateInt[4][3];
};

const ResAnimCurve::Impl::FrameFinder ResAnimCurve::Impl::s_pFuncFindFrame[3] = {
    &ResAnimCurve::FindFrame<float>, &ResAnimCurve::FindFrame<s16>, &ResAnimCurve::FindFrame<u8>};
ResAnimCurve::Impl::FloatEvaluator ResAnimCurve::Impl::s_pFuncEvaluateFloat[3][3] = {
    {&ResAnimCurve::EvaluateCubic<float>, &ResAnimCurve::EvaluateCubic<s16>,
     &ResAnimCurve::EvaluateCubic<s8>},
    {&ResAnimCurve::EvaluateLinear<float>, &ResAnimCurve::EvaluateLinear<s16>,
     &ResAnimCurve::EvaluateLinear<s8>},
    {&ResAnimCurve::EvaluateBakedFloat<float>, &ResAnimCurve::EvaluateBakedFloat<s16>,
     &ResAnimCurve::EvaluateBakedFloat<s8>}};
ResAnimCurve::Impl::IntEvaluator ResAnimCurve::Impl::s_pFuncEvaluateInt[4][3] = {
    {&ResAnimCurve::EvaluateStepInt<int>, &ResAnimCurve::EvaluateStepInt<s16>,
     &ResAnimCurve::EvaluateStepInt<s8>},
    {&ResAnimCurve::EvaluateBakedInt<int>, &ResAnimCurve::EvaluateBakedInt<s16>,
     &ResAnimCurve::EvaluateBakedInt<s8>},
    {&ResAnimCurve::EvaluateStepBool, &ResAnimCurve::EvaluateStepBool, &ResAnimCurve::EvaluateStepBool},
    {&ResAnimCurve::EvaluateBakedBool, &ResAnimCurve::EvaluateBakedBool, &ResAnimCurve::EvaluateBakedBool}};
// frame is the requested sample time; cache supplies the initial key and receives its interval.
template <class T>
void ResAnimCurve::FindFrame(AnimFrameCache* cache, float frame) const {
    using Frame = FrameValue<T>;
    Frame target;

    if (std::is_same<T, s16>::value)
        target = int(std::floor(frame * 32.0f));
    else if (std::is_same<T, u8>::value)
        target = int(frame);
    else
        target = frame;
    const T* values = GetFrameArray<T>();
    const float factor = std::is_same<T, s16>::value ? 0.03125f : 1.0f;

    if (values[keyCount - 1] <= target) {
        cache->keyIndex = u16(keyCount - 1);
        cache->start = float(values[cache->keyIndex]) * factor;
        cache->end = cache->start + 1.0f;
        return;
    }

    ptrdiff_t index = cache->keyIndex;

    if (values[index] > target) {
        do {
            --index;
        } while (values[index] > target);
    } else {
        while (values[index + 1] <= target)
            ++index;
    }

    cache->keyIndex = int(index);
    cache->start = float(values[cache->keyIndex]) * factor;
    cache->end = float(values[cache->keyIndex + 1]) * factor;
}

// cache stores the active key interval; frame selects the interval to locate when it is stale.
void ResAnimCurve::UpdateFrameCache(AnimFrameCache* cache, float frame) const {
    if (cache->start > frame || cache->end <= frame) {
        float inverse = 1.0f / (endFrame - startFrame);
        float ratio = (frame - startFrame) * inverse;
        ratio *= keyCount;
        cache->keyIndex = int(ratio);

        if (keyCount <= unsigned(cache->keyIndex))
            cache->keyIndex = keyCount - 1;
        (this->*Impl::s_pFuncFindFrame[flags & 3])(cache, frame);
    }
}

// frame is sample time; cache reuses the key interval; T specifies the coefficient storage type.
template <class T>
float ResAnimCurve::EvaluateCubic(float frame, AnimFrameCache* cache) const {
    UpdateFrameCache(cache, frame);
    float t = cache->GetInterpolationWeight(frame);
    const T* c = GetKeyArray<T>() + cache->keyIndex * 4;
    float c0 = c[0], c1 = c[1], c2 = c[2], c3 = c[3];
    float high = t * (t * (c2 + t * c3));
    return c0 + t * c1 + high;
}

// frame is sample time; cache reuses the key interval; T specifies the coefficient storage type.
template <class T>
float ResAnimCurve::EvaluateLinear(float frame, AnimFrameCache* cache) const {
    UpdateFrameCache(cache, frame);
    float t = cache->GetInterpolationWeight(frame);
    const T* c = GetKeyArray<T>() + cache->keyIndex * 2;
    return float(c[0]) + t * float(c[1]);
}

// frame selects adjacent baked samples; cache is unused; T specifies the sample storage type.
template <class T>
float ResAnimCurve::EvaluateBakedFloat(float frame, AnimFrameCache*) const {
    int start = int(startFrame);
    int sample = int(frame);
    int index = sample - start;
    float t = frame - sample;
    const T* c = GetKeyArray<T>();
    float c1 = c[index + 1], c0 = c[index];
    return t * c1 + (1.0f - t) * c0;
}

// frame selects a discrete key; cache retains its interval; T specifies the integer storage type.
template <class T>
int ResAnimCurve::EvaluateStepInt(float frame, AnimFrameCache* cache) const {
    UpdateFrameCache(cache, frame);
    return GetKeyArray<T>()[cache->keyIndex];
}

// frame selects a baked integer sample; cache is unused; T specifies the sample storage type.
template <class T>
int ResAnimCurve::EvaluateBakedInt(float frame, AnimFrameCache*) const {
    int start = int(startFrame);
    int index = int(frame) - start;
    return GetKeyArray<T>()[index];
}

// frame selects a packed boolean key and cache retains its interval.
int ResAnimCurve::EvaluateStepBool(float frame, AnimFrameCache* cache) const {
    UpdateFrameCache(cache, frame);
    const u32* values = GetKeyArray<u32>();
    int index = cache->keyIndex;
    return (values[index >> 5] >> (index & 31)) & 1;
}

// frame selects a packed baked boolean sample; cache is unused.
int ResAnimCurve::EvaluateBakedBool(float frame, AnimFrameCache*) const {
    int start = int(startFrame);
    int index = int(frame) - start;
    return (GetKeyArray<u32>()[index >> 5] >> (index & 31)) & 1;
}

namespace {
// frame receives the wrapped time; curve supplies the interval/wrap modes; offset is the initial value bias.
// The returned bias includes any relative-repeat displacement.
template <class T>
inline T WrapFrame(float& frame, const ResAnimCurve* curve, T offset) {
    float start = curve->startFrame;
    float end = curve->endFrame;

    if (start <= frame && frame <= end)
        return offset;
    float distance = start - frame;
    int after;
    unsigned mode;

    if (distance > 0.0f) {
        if (!(curve->flags & 0x300)) {
            frame = start;
            return offset;
        }

        mode = curve->flags >> 8;
        after = 0;
    } else {
        if (!(curve->flags & 0x3000)) {
            frame = end;
            return offset;
        }

        mode = curve->flags >> 12;
        distance = frame - end;
        after = 1;
    }

    float length = end - start;
    int cycles = int(distance / length);
    float remainder = distance - length * cycles;
    mode &= 3;
    int direction;

    switch (mode) {
    case 3: {
        T repetitions = cycles + 1;
        T delta;

        if (std::is_same<T, float>::value)
            delta = curve->deltaFloat;
        else
            delta = curve->deltaInt;
        T shift = -delta * repetitions;

        if (after != 0)
            shift = delta * repetitions;
        offset += shift;
    }

        [[fallthrough]];
    default:
        direction = 1;
        break;
    case 2:
        direction = cycles & 1;
        break;
    }

    frame = end - remainder;

    if (direction == after)
        frame = start + remainder;
    return offset;
}

// value is a byte count/address to round up to pointer alignment.
inline size_t AlignPointer(size_t value) { return (value + 7) & ~size_t(7); }
inline size_t FloatBackupSize() { return AlignPointer(12) + sizeof(void*); }
inline size_t IntBackupSize() { return AlignPointer(4) + sizeof(void*); }
} // namespace

// frame is sample time, including wrap modes; cache retains the last key interval.
float ResAnimCurve::EvaluateFloat(float frame, AnimFrameCache* cache) const {
    float offset = offsetFloat;
    offset = WrapFrame(frame, this, offset);
    float value = (this->*Impl::s_pFuncEvaluateFloat[(flags & 0x70) >> 4][(flags & 12) >> 2])(frame, cache);
    return offset + value * scale;
}

// frame is sample time, including wrap modes; cache retains the last key interval.
int ResAnimCurve::EvaluateInt(float frame, AnimFrameCache* cache) const {
    int offset = offsetInt;
    offset = WrapFrame(frame, this, offset);
    return (this->*Impl::s_pFuncEvaluateInt[((flags & 0x70) - 0x40u) >> 4][(flags & 12) >> 2])(frame, cache) +
           offset;
}

// buffer receives count integer samples; firstFrame is the integer-grid origin and T is storage type.
template <class T>
void ResAnimCurve::BakeImpl(void* buffer, float firstFrame, int count) {
    AnimFrameCache cache;
    T* output = static_cast<T*>(buffer);
    int last = count - 1;
    output[0] = EvaluateStepInt<T>(startFrame, &cache);

    for (int i = 1; i < last; ++i)
        output[i] = EvaluateStepInt<T>(firstFrame + i, &cache);
    output[last] = EvaluateStepInt<T>(endFrame, &cache);
}

// buffer receives count packed boolean samples; firstFrame is the integer-grid origin.
template <>
void ResAnimCurve::BakeImpl<bool>(void* buffer, float firstFrame, int count) {
    AnimFrameCache cache;
    u32* output = static_cast<u32*>(buffer);
    int last = count - 1;
    output[0] = (output[0] & ~1u) | EvaluateStepBool(startFrame, &cache);

    for (int i = 1; i < last; ++i) {
        u32 value = EvaluateStepBool(firstFrame + i, &cache);
        output[unsigned(i) >> 5] = (output[unsigned(i) >> 5] & ~(1u << (i & 31))) | (value << (i & 31));
    }

    u32 value = EvaluateStepBool(endFrame, &cache);
    output[last >> 5] = (output[last >> 5] & ~(1u << (last & 31))) | (value << (last & 31));
}

// buffer receives count floating-point samples; firstFrame is the integer-grid origin.
template <>
void ResAnimCurve::BakeImpl<float>(void* buffer, float firstFrame, int count) {
    AnimFrameCache cache;
    float* output = static_cast<float*>(buffer);
    int last = count - 1;
    output[0] = EvaluateFloat(startFrame, &cache);

    for (int i = 1; i < last; ++i)
        output[i] = EvaluateFloat(firstFrame + i, &cache);
    output[last] = EvaluateFloat(endFrame, &cache);
    float lastFrame = firstFrame + last;
    float fraction = startFrame - firstFrame;
    output[0] = (output[0] - fraction * output[1]) / (1.0f - fraction);
    fraction = lastFrame - endFrame;

    if (fraction < 1.0f)
        output[last] = (output[last] - fraction * output[count - 2]) / (1.0f - fraction);
}

size_t ResAnimCurve::CalculateBakedFloatSize() const {
    int count = GetBakedFloatIntervalCount() + 1;

    if (count < 3)
        return 0;
    static const size_t backupSize = FloatBackupSize();
    return AlignPointer(size_t(count) * sizeof(float)) + backupSize;
}

size_t ResAnimCurve::CalculateBakedIntSize() const {
    int count = GetBakedIntIntervalCount() + 1;

    if (count < 3)
        return 0;
    size_t bytes = 0;

    switch (((flags & 0x70) - 0x40u) >> 4) {
    case 0:
    case 1:
        if ((flags & 12) == 8)
            bytes = count;
        else if ((flags & 12) == 4)
            bytes = count * 2;
        else
            bytes = count * 4;
        break;
    case 2:
    case 3:
        bytes = ((count + 31) & ~31) / 8;
        break;
    }

    static const size_t backupSize = IntBackupSize();
    return backupSize + AlignPointer(bytes);
}

// buffer holds size bytes for samples and the original curve state; size is validated only in debug builds.
void ResAnimCurve::BakeFloat(void* buffer, size_t size) {
    if ((flags & 0x70) == 0x20)
        return;
    int count = GetBakedFloatIntervalCount() + 1;

    if (count < 3)
        return;
    BakeImpl<float>(buffer, std::floor(startFrame), count);
    static const size_t backupSize = FloatBackupSize();
    u8* backup = static_cast<u8*>(buffer) + (CalculateBakedFloatSize() - backupSize);
    *reinterpret_cast<u16*>(backup) = flags;
    *reinterpret_cast<u16*>(backup + 2) = keyCount;
    *reinterpret_cast<float*>(backup + 4) = scale;
    *reinterpret_cast<float*>(backup + 8) = offsetFloat;
    *reinterpret_cast<void**>(AlignPointer(uintptr_t(backup + 12))) = keys;
    flags = (flags & ~0x7c) | 0x20;
    keyCount = count;
    scale = 1.0f;
    offsetFloat = 0.0f;
    keys = buffer;
}

// buffer holds size bytes for samples and the original curve state; size is validated only in debug builds.
void ResAnimCurve::BakeInt(void* buffer, size_t size) {
    unsigned type = flags & 0x70;

    if (type == 0x20 || type == 0x70)
        return;
    int count = GetBakedIntIntervalCount() + 1;

    if (count < 3)
        return;
    float first = std::floor(startFrame);
    u16 bakedFlags = 0;

    if (type == 0x40) {
        if ((flags & 12) == 8)
            BakeImpl<s8>(buffer, first, count);
        else if ((flags & 12) == 4)
            BakeImpl<s16>(buffer, first, count);
        else
            BakeImpl<int>(buffer, first, count);
        bakedFlags = (flags & ~0x70) | 0x50;
    } else if (type == 0x60) {
        BakeImpl<bool>(buffer, first, count);
        bakedFlags = 0x70;
    }

    static const size_t backupSize = IntBackupSize();
    u8* backup = static_cast<u8*>(buffer) + (CalculateBakedIntSize() - backupSize);
    *reinterpret_cast<u16*>(backup) = flags;
    *reinterpret_cast<u16*>(backup + 2) = keyCount;
    *reinterpret_cast<void**>(AlignPointer(uintptr_t(backup + 4))) = keys;
    flags = bakedFlags;
    keyCount = count;
    keys = buffer;
}

void ResAnimCurve::ResetFloat() {
    if ((flags & 0x70) != 0x20)
        return;
    u8* buffer = static_cast<u8*>(keys);
    static const size_t backupSize = FloatBackupSize();
    u8* backup = buffer + (CalculateBakedFloatSize() - backupSize);
    flags = *reinterpret_cast<u16*>(backup);
    keyCount = *reinterpret_cast<u16*>(backup + 2);
    scale = *reinterpret_cast<float*>(backup + 4);
    offsetFloat = *reinterpret_cast<float*>(backup + 8);
    keys = *reinterpret_cast<void**>(AlignPointer(uintptr_t(backup + 12)));
}

void ResAnimCurve::ResetInt() {
    if ((flags & 0x50) != 0x50)
        return;
    u8* buffer = static_cast<u8*>(keys);
    static const size_t backupSize = IntBackupSize();
    u8* backup = buffer + (CalculateBakedIntSize() - backupSize);
    flags = *reinterpret_cast<u16*>(backup);
    keyCount = *reinterpret_cast<u16*>(backup + 2);
    keys = *reinterpret_cast<void**>(AlignPointer(uintptr_t(backup + 4)));
}
} // namespace nn::g3d

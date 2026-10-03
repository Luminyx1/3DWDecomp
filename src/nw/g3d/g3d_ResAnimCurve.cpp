#include <nn/g3d/g3d_ResAnimCurve.h>
#include <cmath>
#include <cstddef>
#include <type_traits>

namespace nn::g3d {
template <class T>
using FrameValue = typename std::conditional<
    std::is_same<T, float>::value, float,
    typename std::conditional<std::is_same<T, s16>::value, int, unsigned>::type>::type;

/**
 * @brief Count frame-grid intervals needed for floating-point baking.
 * @return Interval count including the extra upper interpolation sample.
 */
inline int ResAnimCurve::GetBakedFloatIntervalCount() const {
    return static_cast<int>(std::floor(endFrame) + 1.0f) - static_cast<int>(std::floor(startFrame));
}

/**
 * @brief Count frame-grid intervals needed for discrete baking.
 * @return Number of intervals between the floored endpoints.
 */
inline int ResAnimCurve::GetBakedIntIntervalCount() const {
    return static_cast<int>(std::floor(endFrame)) - static_cast<int>(std::floor(startFrame));
}

struct ResAnimCurve::Impl {
    using FrameFinder = void (ResAnimCurve::*)(AnimFrameCache*, float) const;
    using FloatEvaluator = float (ResAnimCurve::*)(float, AnimFrameCache* cache) const;
    using IntEvaluator = int (ResAnimCurve::*)(float, AnimFrameCache* cache) const;
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
/**
 * @brief Locate the key interval containing a sample time.
 * @tparam T Stored frame representation: float, signed fixed-point short, or unsigned byte.
 * @param cache Non-null cache supplying the initial key index and receiving the interval.
 * @param frame Sample time within the curve range; quantized frames use their stored comparison domain.
 */
template <class T> void ResAnimCurve::FindFrame(AnimFrameCache* cache, float frame) const {
    using Frame = FrameValue<T>;
    const T* values = GetFrameArray<T>();
    Frame target;

    if (std::is_same<T, s16>::value) {
        target = static_cast<int>(std::floor(frame * 32.0f));
    } else if (std::is_same<T, u8>::value) {
        target = static_cast<int>(frame);
    } else {
        target = frame;
    }
    const float factor = std::is_same<T, s16>::value ? 0.03125f : 1.0f;

    if (values[keyCount - 1] <= target) {
        cache->keyIndex = static_cast<u16>(keyCount - 1);
        cache->start = static_cast<float>(values[cache->keyIndex]) * factor;
        cache->end = cache->start + 1.0f;
        return;
    }

    ptrdiff_t index = cache->keyIndex;

    if (values[index] > target) {
        do {
            --index;
        } while (values[index] > target);
    } else {
        while (values[index + 1] <= target) {
            ++index;
        }
    }

    cache->keyIndex = static_cast<int>(index);
    cache->start = static_cast<float>(values[cache->keyIndex]) * factor;
    cache->end = static_cast<float>(values[cache->keyIndex + 1]) * factor;
}

/**
 * @brief Refresh a cached key interval when the sample leaves its bounds.
 * @param cache Non-null cache initialized for this curve.
 * @param frame Requested sample time within the curve range.
 */
void ResAnimCurve::UpdateFrameCache(AnimFrameCache* cache, float frame) const {
    if (cache->start > frame || cache->end <= frame) {
        float ratio = GetNormalizedFrame(frame);
        ratio *= keyCount;
        cache->keyIndex = static_cast<int>(ratio);

        if (keyCount <= static_cast<unsigned>(cache->keyIndex)) {
            cache->keyIndex = keyCount - 1;
        }
        (this->*Impl::s_pFuncFindFrame[flags & 3])(cache, frame);
    }
}

/**
 * @brief Evaluate cubic coefficients for the active key interval.
 * @tparam T Stored key or sample representation; controls conversion and element width.
 * @param frame Sample time within the curve range.
 * @param cache Non-null cache retaining the active key interval.
 * @return Interpolated value before the curve scale and offset.
 */
template <class T> float ResAnimCurve::EvaluateCubic(float frame, AnimFrameCache* cache) const {
    UpdateFrameCache(cache, frame);
    float t = cache->GetInterpolationWeight(frame);
    const T* c = GetKeyArray<T>() + cache->keyIndex * 4;
    float c0 = c[0], c1 = c[1], c2 = c[2], c3 = c[3];
    float high = t * (t * (c2 + t * c3));
    return c0 + t * c1 + high;
}

/**
 * @brief Evaluate linear coefficients for the active key interval.
 * @tparam T Stored key or sample representation; controls conversion and element width.
 * @param frame Sample time within the curve range.
 * @param cache Non-null cache retaining the active key interval.
 * @return Interpolated value before the curve scale and offset.
 */
template <class T> float ResAnimCurve::EvaluateLinear(float frame, AnimFrameCache* cache) const {
    UpdateFrameCache(cache, frame);
    float t = cache->GetInterpolationWeight(frame);
    const T* c = GetKeyArray<T>() + cache->keyIndex * 2;
    return static_cast<float>(c[0]) + t * static_cast<float>(c[1]);
}

/**
 * @brief Interpolate adjacent samples in a baked floating-point curve.
 * @tparam T Stored key or sample representation; controls conversion and element width.
 * @param frame Sample time with two available adjacent baked samples.
 * @param cache Unused; baked samples require no key-interval lookup.
 * @return Interpolated baked value.
 */
template <class T> float ResAnimCurve::EvaluateBakedFloat(float frame, AnimFrameCache* cache) const {
    int start = static_cast<int>(startFrame);
    int sample = static_cast<int>(frame);
    int index = sample - start;
    float t = frame - sample;
    const T* c = GetKeyArray<T>();
    float c1 = c[index + 1], c0 = c[index];
    return t * c1 + (1.0f - t) * c0;
}

/**
 * @brief Read the discrete value for the active key interval.
 * @tparam T Stored key or sample representation; controls conversion and element width.
 * @param frame Sample time within the curve range.
 * @param cache Non-null cache retaining the active key interval.
 * @return Stored integer value.
 */
template <class T> int ResAnimCurve::EvaluateStepInt(float frame, AnimFrameCache* cache) const {
    UpdateFrameCache(cache, frame);
    return GetKeyArray<T>()[cache->keyIndex];
}

/**
 * @brief Read a baked integer sample on the frame grid.
 * @tparam T Stored key or sample representation; controls conversion and element width.
 * @param frame Sample time selecting a valid baked frame.
 * @param cache Unused; baked samples require no key-interval lookup.
 * @return Stored integer sample.
 */
template <class T> int ResAnimCurve::EvaluateBakedInt(float frame, AnimFrameCache* cache) const {
    int start = static_cast<int>(startFrame);
    int index = static_cast<int>(frame) - start;
    return GetKeyArray<T>()[index];
}

/**
 * @brief Read the packed boolean for the active key interval.
 * @param frame Sample time within the curve range.
 * @param cache Non-null cache retaining the active key interval.
 * @return Zero or one from the packed key bit.
 */
int ResAnimCurve::EvaluateStepBool(float frame, AnimFrameCache* cache) const {
    UpdateFrameCache(cache, frame);
    const u32* values = GetKeyArray<u32>();
    int index = cache->keyIndex;
    return (values[index >> 5] >> (index & 31)) & 1;
}

/**
 * @brief Read a packed baked boolean on the frame grid.
 * @param frame Sample time selecting a valid baked frame.
 * @param cache Unused; baked samples require no key-interval lookup.
 * @return Zero or one from the packed sample bit.
 */
int ResAnimCurve::EvaluateBakedBool(float frame, AnimFrameCache* cache) const {
    int start = static_cast<int>(startFrame);
    int index = static_cast<int>(frame) - start;
    return (GetKeyArray<u32>()[index >> 5] >> (index & 31)) & 1;
}

namespace {
/**
 * @brief Wrap or clamp a sample time and accumulate relative-repeat bias.
 * @tparam T Value-bias type, either float or int.
 * @param frame Sample time, updated to the curve interval according to its wrap modes.
 * @param curve Non-null curve supplying endpoints, wrap modes and repeat deltas.
 * @param offset Initial value bias to which relative-repeat displacement is added.
 * @return Adjusted value bias.
 */
template <class T> inline T WrapFrame(float& frame, const ResAnimCurve* curve, T offset) {
    float start = curve->startFrame;
    float end = curve->endFrame;

    if (start <= frame && frame <= end) {
        return offset;
    }
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
    int cycles = static_cast<int>(distance / length);
    float remainder = distance - length * cycles;
    mode &= 3;
    int direction;

    switch (mode) {
    case 3: {
        T repetitions = cycles + 1;
        T delta;

        if (std::is_same<T, float>::value) {
            delta = curve->deltaFloat;
        } else {
            delta = curve->deltaInt;
        }
        T shift = -delta * repetitions;

        if (after != 0) {
            shift = delta * repetitions;
        }
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

    if (direction == after) {
        frame = start + remainder;
    }
    return offset;
}

/**
 * @brief Round an address or byte count up to eight-byte alignment.
 * @param value Address or byte count, with room for alignment padding.
 * @return Smallest aligned value not less than the input.
 */
inline size_t AlignPointer(size_t value) { return (value + 7) & ~static_cast<size_t>(7); }
/**
 * @brief Calculate storage for the original floating-point curve state.
 * @return Backup size including the aligned key-array pointer.
 */
inline size_t FloatBackupSize() { return AlignPointer(12) + sizeof(void*); }
/**
 * @brief Calculate storage for the original discrete curve state.
 * @return Backup size including the aligned key-array pointer.
 */
inline size_t IntBackupSize() { return AlignPointer(4) + sizeof(void*); }
struct CurveBackupHeader {
    u16 flags;
    u16 keyCount;
};
struct FloatCurveBackup : CurveBackupHeader {
    float scale;
    float offset;
};
static_assert(sizeof(CurveBackupHeader) == 4);
static_assert(sizeof(FloatCurveBackup) == 12);
/**
 * @brief Locate the typed original-state backup after a baked sample array.
 * @tparam T Header layout for the floating-point or discrete curve backup.
 * @param pBuffer Start of the caller-owned sample and backup storage.
 * @param totalSize Total size calculated for this curve's baked storage.
 * @param backupSize Size reserved for the backup, no greater than totalSize.
 * @return Pointer to the backup header within the supplied buffer.
 */
template <class T> inline T* GetCurveBackup(void* pBuffer, size_t totalSize, size_t backupSize) {
    return reinterpret_cast<T*>(static_cast<u8*>(pBuffer) + (totalSize - backupSize));
}
/**
 * @brief Access the saved key-array pointer after an aligned backup header.
 * @tparam T Backup header layout whose size precedes the pointer slot.
 * @param pBackup Non-null header followed by writable pointer-aligned storage.
 * @return Reference to the saved original key-array pointer.
 */
template <class T> inline void*& GetBackupKeys(T* pBackup) {
    return *reinterpret_cast<void**>(AlignPointer(reinterpret_cast<uintptr_t>(pBackup) + sizeof(T)));
}
} // namespace

/**
 * @brief Evaluate a floating-point curve with wrapping, scale and offset.
 * @param frame Requested sample time; out-of-range values follow the curve wrap modes.
 * @param cache Non-null cache retaining the last key interval.
 * @return Final animated floating-point value.
 */
float ResAnimCurve::EvaluateFloat(float frame, AnimFrameCache* cache) const {
    float offset = offsetFloat;
    offset = WrapFrame(frame, this, offset);
    float value = (this->*Impl::s_pFuncEvaluateFloat[(flags & 0x70) >> 4][(flags & 12) >> 2])(frame, cache);
    return offset + value * scale;
}

/**
 * @brief Evaluate a discrete curve with wrapping and value offset.
 * @param frame Requested sample time; out-of-range values follow the curve wrap modes.
 * @param cache Non-null cache retaining the last key interval.
 * @return Final animated integer or boolean value.
 */
int ResAnimCurve::EvaluateInt(float frame, AnimFrameCache* cache) const {
    int offset = offsetInt;
    offset = WrapFrame(frame, this, offset);
    return (this->*Impl::s_pFuncEvaluateInt[((flags & 0x70) - 0x40u) >> 4][(flags & 12) >> 2])(frame, cache) +
           offset;
}

/**
 * @brief Write curve samples on the integer frame grid.
 * @tparam T Stored key or sample representation; controls conversion and element width.
 * @param buffer Writable sample storage, packed into words for boolean curves.
 * @param firstFrame Floored starting frame defining the integer sample grid.
 * @param count Number of samples, at least three; endpoints use the exact curve bounds.
 */
template <class T> void ResAnimCurve::BakeImpl(void* buffer, float firstFrame, int count) {
    AnimFrameCache cache;
    T* output = static_cast<T*>(buffer);
    int last = count - 1;
    output[0] = EvaluateStepInt<T>(startFrame, &cache);

    for (int i = 1; i < last; ++i) {
        output[i] = EvaluateStepInt<T>(firstFrame + i, &cache);
    }
    output[last] = EvaluateStepInt<T>(endFrame, &cache);
}

/**
 * @brief Write curve samples on the integer frame grid.
 * @param buffer Writable sample storage, packed into words for boolean curves.
 * @param firstFrame Floored starting frame defining the integer sample grid.
 * @param count Number of samples, at least three; endpoints use the exact curve bounds.
 */
template <> void ResAnimCurve::BakeImpl<bool>(void* buffer, float firstFrame, int count) {
    AnimFrameCache cache;
    u32* output = static_cast<u32*>(buffer);
    int last = count - 1;
    output[0] = (output[0] & ~1u) | EvaluateStepBool(startFrame, &cache);

    for (int i = 1; i < last; ++i) {
        u32 value = EvaluateStepBool(firstFrame + i, &cache);
        output[static_cast<unsigned>(i) >> 5] =
            (output[static_cast<unsigned>(i) >> 5] & ~(1u << (i & 31))) | (value << (i & 31));
    }

    u32 value = EvaluateStepBool(endFrame, &cache);
    output[last >> 5] = (output[last >> 5] & ~(1u << (last & 31))) | (value << (last & 31));
}

/**
 * @brief Write curve samples on the integer frame grid.
 * @param buffer Writable sample storage, packed into words for boolean curves.
 * @param firstFrame Floored starting frame defining the integer sample grid.
 * @param count Number of samples, at least three; endpoints use the exact curve bounds.
 */
template <> void ResAnimCurve::BakeImpl<float>(void* buffer, float firstFrame, int count) {
    AnimFrameCache cache;
    float* output = static_cast<float*>(buffer);
    int last = count - 1;
    output[0] = EvaluateFloat(startFrame, &cache);

    for (int i = 1; i < last; ++i) {
        output[i] = EvaluateFloat(firstFrame + i, &cache);
    }
    output[last] = EvaluateFloat(endFrame, &cache);
    float lastFrame = firstFrame + last;
    float fraction = startFrame - firstFrame;
    output[0] = (output[0] - fraction * output[1]) / (1.0f - fraction);
    fraction = lastFrame - endFrame;

    if (fraction < 1.0f) {
        output[last] = (output[last] - fraction * output[count - 2]) / (1.0f - fraction);
    }
}

/**
 * @brief Calculate the sample and backup storage needed for floating-point baking.
 * @return Required byte count, or zero when the interval has too few samples.
 */
size_t ResAnimCurve::CalculateBakedFloatSize() const {
    int count = GetBakedFloatIntervalCount() + 1;

    if (count < 3) {
        return 0;
    }
    static const size_t backupSize = FloatBackupSize();
    return AlignPointer(static_cast<size_t>(count) * sizeof(float)) + backupSize;
}

/**
 * @brief Calculate the sample and backup storage needed for discrete baking.
 * @return Required byte count, or zero when the interval has too few samples.
 */
size_t ResAnimCurve::CalculateBakedIntSize() const {
    int count = GetBakedIntIntervalCount() + 1;

    if (count < 3) {
        return 0;
    }
    size_t bytes = 0;

    switch (((flags & 0x70) - 0x40u) >> 4) {
    case 0:
    case 1:
        if ((flags & 12) == 8) {
            bytes = count;
        } else if ((flags & 12) == 4) {
            bytes = count * 2;
        } else {
            bytes = count * 4;
        }
        break;
    case 2:
    case 3:
        bytes = ((count + 31) & ~31) / 8;
        break;
    }

    static const size_t backupSize = IntBackupSize();
    return backupSize + AlignPointer(bytes);
}

/**
 * @brief Replace floating-point keys with baked samples while saving the original state.
 * @param buffer Writable storage of at least CalculateBakedFloatSize() bytes.
 * @param size Available storage size; unused in this release build.
 */
void ResAnimCurve::BakeFloat(void* buffer, size_t size) {
    if ((flags & 0x70) == 0x20) {
        return;
    }
    int count = GetBakedFloatIntervalCount() + 1;

    if (count < 3) {
        return;
    }
    BakeImpl<float>(buffer, std::floor(startFrame), count);
    static const size_t backupSize = FloatBackupSize();
    auto* backup = GetCurveBackup<FloatCurveBackup>(buffer, CalculateBakedFloatSize(), backupSize);
    backup->flags = flags;
    backup->keyCount = keyCount;
    backup->scale = scale;
    backup->offset = offsetFloat;
    GetBackupKeys(backup) = keys;
    flags = (flags & ~0x7c) | 0x20;
    keyCount = count;
    scale = 1.0f;
    offsetFloat = 0.0f;
    keys = buffer;
}

/**
 * @brief Replace discrete keys with baked samples while saving the original state.
 * @param buffer Writable storage of at least CalculateBakedIntSize() bytes.
 * @param size Available storage size; unused in this release build.
 */
void ResAnimCurve::BakeInt(void* buffer, size_t size) {
    unsigned type = flags & 0x70;

    if (type == 0x20 || type == 0x70) {
        return;
    }
    int count = GetBakedIntIntervalCount() + 1;

    if (count < 3) {
        return;
    }
    float first = std::floor(startFrame);
    u16 bakedFlags = 0;

    if (type == 0x40) {
        if ((flags & 12) == 8) {
            BakeImpl<s8>(buffer, first, count);
        } else if ((flags & 12) == 4) {
            BakeImpl<s16>(buffer, first, count);
        } else {
            BakeImpl<int>(buffer, first, count);
        }
        bakedFlags = (flags & ~0x70) | 0x50;
    } else if (type == 0x60) {
        BakeImpl<bool>(buffer, first, count);
        bakedFlags = 0x70;
    }

    static const size_t backupSize = IntBackupSize();
    auto* backup = GetCurveBackup<CurveBackupHeader>(buffer, CalculateBakedIntSize(), backupSize);
    backup->flags = flags;
    backup->keyCount = keyCount;
    GetBackupKeys(backup) = keys;
    flags = bakedFlags;
    keyCount = count;
    keys = buffer;
}

/**
 * @brief Restore the original floating-point keys and parameters from their baked backup.
 */
void ResAnimCurve::ResetFloat() {
    if ((flags & 0x70) != 0x20) {
        return;
    }
    u8* buffer = static_cast<u8*>(keys);
    static const size_t backupSize = FloatBackupSize();
    auto* backup = GetCurveBackup<FloatCurveBackup>(buffer, CalculateBakedFloatSize(), backupSize);
    flags = backup->flags;
    keyCount = backup->keyCount;
    scale = backup->scale;
    offsetFloat = backup->offset;
    keys = GetBackupKeys(backup);
}

/**
 * @brief Restore the original discrete keys and flags from their baked backup.
 */
void ResAnimCurve::ResetInt() {
    if ((flags & 0x50) != 0x50) {
        return;
    }
    u8* buffer = static_cast<u8*>(keys);
    static const size_t backupSize = IntBackupSize();
    auto* backup = GetCurveBackup<CurveBackupHeader>(buffer, CalculateBakedIntSize(), backupSize);
    flags = backup->flags;
    keyCount = backup->keyCount;
    keys = GetBackupKeys(backup);
}
} // namespace nn::g3d

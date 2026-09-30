#include <nerd/nerdMath.h>
#include <cmath>

namespace nerd {
namespace {
bool sUseFastsqrte;
const u16 cEstimateBase[32] = {
    27130, 24366, 21834, 19504, 17352, 15356, 13496, 11760,
    10132, 8604, 7164, 5806, 4520, 3302, 2146, 1046,
    65512, 61604, 58024, 54728, 51684, 48860, 46232, 43776,
    41476, 39316, 37280, 35356, 33540, 31816, 30180, 28624
};
const u16 cEstimateSlope[32] = {
    5536, 5068, 4660, 4308, 3996, 3720, 3476, 3256,
    3056, 2880, 2720, 2572, 2436, 2316, 2200, 2092,
    7824, 7168, 6592, 6088, 5648, 5264, 4912, 4600,
    4328, 4072, 3848, 3640, 3448, 3272, 3112, 2968
};

/**
 * Computes the table-based double-precision reciprocal-square-root estimate.
 * @param value Input value, including signed zero and non-finite values.
 * @return The estimate before single-precision refinement.
 */
double estimateReciprocalSqrt(double value)
{
    u64 bits;
    __builtin_memcpy(&bits, &value, sizeof(bits));
    const u64 magnitude = bits & 0x7fffffffffffffffULL;
    if (magnitude > 0x7ff0000000000000ULL)
        bits |= 0x0008000000000000ULL;
    else if (magnitude == 0)
        bits = 0;
    else if (bits >> 63)
        bits = 0x7ff8000000000000ULL;
    else if (magnitude == 0x7ff0000000000000ULL)
        bits = 0;
    else {
        u32 exponent;
        u32 index;
        if (bits & 0x7ff0000000000000ULL) {
            exponent = 0x5fe - ((static_cast<u32>(bits >> 52) + 1) >> 1);
            index = (bits >> 37) & 0xffff;
        } else {
            u64 mask = 0x8000000000000000ULL;
            int shift = -12;
            do {
                ++shift;
            } while (static_cast<u32>(shift + 12) < 64 && !(bits & (mask >>= 1)));
            index = static_cast<u32>((bits << (shift + 1)) >> 37);
            index &= ((shift & 1) ? 0xffff : 0x7fff);
            exponent = 0x5fe + (static_cast<u32>(shift) >> 1);
        }
        const u32 tableIndex = static_cast<u32>(index) >> 11;
        bits = (static_cast<u64>(cEstimateBase[tableIndex]) << 36) -
               (static_cast<u64>(index & 0x7ff) * cEstimateSlope[tableIndex] << 24);
        bits |= static_cast<u64>(exponent) << 52;
    }
    __builtin_memcpy(&value, &bits, sizeof(value));
    return value;
}
}

/**
 * Selects the table-based square-root approximation.
 * @param enabled Whether to use the fast estimate and refinement path.
 */
void setUseFastsqrte(bool enabled) { sUseFastsqrte = enabled; }

/**
 * Computes a square root using the selected implementation.
 * @param value Input whose square root is requested.
 * @return The square root, or the fast-path approximation when enabled.
 */
f32 sqrt(f32 value)
{
    if (sUseFastsqrte)
        return rsqrt(value) * value;
    return std::sqrt(value);
}

/**
 * Computes a reciprocal square root using the selected implementation.
 * @param value Input whose reciprocal square root is requested.
 * @return The reciprocal square root, with original fast-path special-value behavior.
 */
// NON_MATCHING: subnormal normalization loop and one index-mask instruction differ.
f32 rsqrt(f32 value)
{
    if (!sUseFastsqrte)
        return 1.0f / std::sqrt(value);
    const f32 estimate = estimateReciprocalSqrt(value);
    const f32 square = estimate * estimate;
    return (estimate * -0.5f) * (square * value - 3.0f);
}
}  // namespace nerd

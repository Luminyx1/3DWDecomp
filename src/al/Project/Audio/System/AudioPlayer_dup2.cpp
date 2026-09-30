#include <cmath>

#include "Library/Se/Project/SeCategory.hpp"

namespace al {
/**
 * Converts a volume in decibels to a volume ratio.
 * @param decibel Volume in decibels.
 * @return Volume ratio, or 0 at or below the lowest volume.
 */
f32 calcDecibelToRatio(f32 decibel) {
    if (decibel <= -96.3f) {
        return 0.0f;
    }

    return powf(10.0f, decibel * 0.05f);
}

/**
 * Converts a volume ratio to a volume in decibels.
 * @param ratio Volume ratio.
 * @return Volume in decibels, or the lowest volume for a ratio of 0 or less.
 */
f32 calcRatioToDecibel(f32 ratio) {
    if (ratio <= 0.0f) {
        return -96.3f;
    }

    return log10f(ratio) * 20.0f;
}
}  // namespace al

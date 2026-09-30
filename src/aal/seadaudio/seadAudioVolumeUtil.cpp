#include "audio/seadAudioVolumeUtil.h"

#include "math/seadMathCalcCommon.h"

namespace sead {
/**
 * Converts a decibel value to a linear volume ratio.
 * @param decibel Volume in decibels.
 * @return Linear volume ratio (0 below -90.309 dB).
 */
f32 AudioVolumeUtil::calcVolumeRatioFromDecibel(f32 decibel) {
    if (decibel < -90.309f) {
        return 0.0f;
    }

    return Mathf::pow(10.0f, decibel * 0.05f);
}

/**
 * Converts a linear volume ratio to decibels.
 * @param ratio Linear volume ratio.
 * @return Volume in decibels (-90.4 for non-positive ratios).
 */
f32 AudioVolumeUtil::calcDecibelFromVolumeRatio(f32 ratio) {
    if (ratio <= 0.0f) {
        return -90.4f;
    }

    return Mathf::log10(ratio) * 20.0f;
}

/**
 * Converts a millibel value to a linear volume ratio.
 * @param millibel Volume in millibels.
 * @return Linear volume ratio (0 below -9030.899 mB).
 */
f32 AudioVolumeUtil::calcVolumeRatioFromMillibel(f32 millibel) {
    if (millibel < -9030.899f) {
        return 0.0f;
    }

    return Mathf::pow(10.0f, millibel * 0.0005f);
}

/**
 * Converts a linear volume ratio to millibels.
 * @param ratio Linear volume ratio.
 * @return Volume in millibels (-9030.9 for non-positive ratios).
 */
f32 AudioVolumeUtil::calcMillibelFromVolumeRatio(f32 ratio) {
    if (ratio <= 0.0f) {
        return -9030.9f;
    }

    return Mathf::log10(ratio) * 2000.0f;
}

/**
 * Converts a decibel value to a linear volume ratio using the exp/log tables.
 * @param decibel Volume in decibels.
 * @return Linear volume ratio (0 below -90.309 dB).
 */
f32 AudioVolumeUtil::calcVolumeRatioFromDecibelTable(f32 decibel) {
    if (decibel < -90.309f) {
        return 0.0f;
    }

    return Mathf::expTable(decibel * 0.05f * Mathf::logTable(10.0f));
}

/**
 * Converts a linear volume ratio to decibels using the log table.
 * @param ratio Linear volume ratio.
 * @return Volume in decibels (-90.4 for non-positive ratios).
 */
f32 AudioVolumeUtil::calcDecibelFromVolumeRatioTable(f32 ratio) {
    if (ratio <= 0.0f) {
        return -90.4f;
    }

    return Mathf::logTable(ratio) * 8.6858896f;
}

/**
 * Converts a millibel value to a linear volume ratio using the exp/log tables.
 * @param millibel Volume in millibels.
 * @return Linear volume ratio (0 below -9030.899 mB).
 */
f32 AudioVolumeUtil::calcVolumeRatioFromMillibelTable(f32 millibel) {
    if (millibel < -9030.899f) {
        return 0.0f;
    }

    return Mathf::expTable(millibel * 0.0005f * Mathf::logTable(10.0f));
}

/**
 * Converts a linear volume ratio to millibels using the log table.
 * @param ratio Linear volume ratio.
 * @return Volume in millibels (-9030.9 for non-positive ratios).
 */
f32 AudioVolumeUtil::calcMillibelFromVolumeRatioTable(f32 ratio) {
    if (ratio <= 0.0f) {
        return -9030.9f;
    }

    return Mathf::logTable(ratio) * 868.58893f;
}
}  // namespace sead

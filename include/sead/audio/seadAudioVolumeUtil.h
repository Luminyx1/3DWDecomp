#pragma once

#include <basis/seadTypes.h>

namespace sead {
class AudioVolumeUtil {
public:
    static f32 calcVolumeRatioFromDecibel(f32 decibel);
    static f32 calcDecibelFromVolumeRatio(f32 ratio);
    static f32 calcVolumeRatioFromMillibel(f32 millibel);
    static f32 calcMillibelFromVolumeRatio(f32 ratio);
    static f32 calcVolumeRatioFromDecibelTable(f32 decibel);
    static f32 calcDecibelFromVolumeRatioTable(f32 ratio);
    static f32 calcVolumeRatioFromMillibelTable(f32 millibel);
    static f32 calcMillibelFromVolumeRatioTable(f32 ratio);
};
}  // namespace sead

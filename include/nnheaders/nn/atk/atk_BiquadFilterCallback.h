#pragma once
#include <nn/types.h>

namespace nn::atk {
struct BiquadFilterCoefficients {
    s16 numerator[3];
    s16 denominator[2];
};
static_assert(sizeof(BiquadFilterCoefficients) == 10, "Biquad coefficient size");

class BiquadFilterCallback {
  public:
    virtual ~BiquadFilterCallback();
    /**
     * @brief Calculate coefficients for a filter setting.
     * @param pCoefficients Non-null destination for the five filter coefficients.
     * @param sampleRate Playback sample rate in hertz; table-based presets may ignore it.
     * @param value Filter control value used by the selected preset.
     */
    virtual void GetCoefficients(BiquadFilterCoefficients* pCoefficients, int sampleRate,
                                 float value) const = 0;
};
} // namespace nn::atk

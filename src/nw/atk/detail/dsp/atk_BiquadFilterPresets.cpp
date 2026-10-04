#include <nn/atk/atk_BiquadFilterPresets.h>

namespace nn::atk::detail {
namespace {
/**
 * @brief Select coefficients from a preset table using a bounded control value.
 * @tparam Count Number of entries in the coefficient table; must exceed zero.
 * @param pCoefficients Non-null destination for the selected coefficients.
 * @param rTable Coefficient table ordered by increasing control value.
 * @param value Control value scaled to the table's index range, then clamped.
 */
template <size_t Count>
inline void ReadCoefficients(BiquadFilterCoefficients* pCoefficients,
                             const BiquadFilterCoefficients (&rTable)[Count], float value) {
    int index = static_cast<int>(value * (Count - 1));
    if (index < 0) {
        index = 0;
    }
    if (index >= static_cast<int>(Count - 1)) {
        index = Count - 1;
    }
    *pCoefficients = rTable[static_cast<unsigned>(index)];
}
} // namespace
/**
 * @brief Select coefficients for the low-pass preset.
 * @param pCoefficients Non-null destination for the five filter coefficients.
 * @param sampleRate Unused; this preset uses its fixed 32000 Hz coefficient table.
 * @param value Filter control value, normally between zero and one; the table index is clamped.
 */
void BiquadFilterLpf::GetCoefficients(BiquadFilterCoefficients* pCoefficients, int sampleRate,
                                      float value) const {
    ReadCoefficients(pCoefficients, CoefficientsTable32000, value);
}

/**
 * @brief Select coefficients for the high-pass preset.
 * @param pCoefficients Non-null destination for the five filter coefficients.
 * @param sampleRate Unused; this preset uses its fixed 32000 Hz coefficient table.
 * @param value Filter control value, normally between zero and one; the table index is clamped.
 */
void BiquadFilterHpf::GetCoefficients(BiquadFilterCoefficients* pCoefficients, int sampleRate,
                                      float value) const {
    ReadCoefficients(pCoefficients, CoefficientsTable32000, value);
}

/**
 * @brief Select coefficients for the 512 Hz band-pass preset.
 * @param pCoefficients Non-null destination for the five filter coefficients.
 * @param sampleRate Unused; this preset uses its fixed 32000 Hz coefficient table.
 * @param value Filter control value, normally between zero and one; the table index is clamped.
 */
void BiquadFilterBpf512::GetCoefficients(BiquadFilterCoefficients* pCoefficients, int sampleRate,
                                         float value) const {
    ReadCoefficients(pCoefficients, CoefficientsTable32000, (2.0f - value) * value);
}

/**
 * @brief Select coefficients for the 1024 Hz band-pass preset.
 * @param pCoefficients Non-null destination for the five filter coefficients.
 * @param sampleRate Unused; this preset uses its fixed 32000 Hz coefficient table.
 * @param value Filter control value, normally between zero and one; the table index is clamped.
 */
void BiquadFilterBpf1024::GetCoefficients(BiquadFilterCoefficients* pCoefficients, int sampleRate,
                                          float value) const {
    ReadCoefficients(pCoefficients, CoefficientsTable32000, (2.0f - value) * value);
}

/**
 * @brief Select coefficients for the 2048 Hz band-pass preset.
 * @param pCoefficients Non-null destination for the five filter coefficients.
 * @param sampleRate Unused; this preset uses its fixed 32000 Hz coefficient table.
 * @param value Filter control value, normally between zero and one; the table index is clamped.
 */
void BiquadFilterBpf2048::GetCoefficients(BiquadFilterCoefficients* pCoefficients, int sampleRate,
                                          float value) const {
    ReadCoefficients(pCoefficients, CoefficientsTable32000, (2.0f - value) * value);
}

/**
 * @brief Select coefficients for the low-pass preset with 48 kHz compatibility.
 * @param pCoefficients Non-null destination for the five filter coefficients.
 * @param sampleRate Unused; this preset uses its fixed 48000 Hz coefficient table.
 * @param value Filter control value, normally between zero and one; the table index is clamped.
 */
void BiquadFilterLpfNw4fCompatible48k::GetCoefficients(BiquadFilterCoefficients* pCoefficients,
                                                       int sampleRate, float value) const {
    ReadCoefficients(pCoefficients, CoefficientsTable48000, value);
}

/**
 * @brief Select coefficients for the high-pass preset with 48 kHz compatibility.
 * @param pCoefficients Non-null destination for the five filter coefficients.
 * @param sampleRate Unused; this preset uses its fixed 48000 Hz coefficient table.
 * @param value Filter control value, normally between zero and one; the table index is clamped.
 */
void BiquadFilterHpfNw4fCompatible48k::GetCoefficients(BiquadFilterCoefficients* pCoefficients,
                                                       int sampleRate, float value) const {
    ReadCoefficients(pCoefficients, CoefficientsTable48000, value);
}

/**
 * @brief Select coefficients for the 512 Hz band-pass preset with 48 kHz compatibility.
 * @param pCoefficients Non-null destination for the five filter coefficients.
 * @param sampleRate Unused; this preset uses its fixed 48000 Hz coefficient table.
 * @param value Filter control value, normally between zero and one; the table index is clamped.
 */
void BiquadFilterBpf512Nw4fCompatible48k::GetCoefficients(BiquadFilterCoefficients* pCoefficients,
                                                          int sampleRate, float value) const {
    ReadCoefficients(pCoefficients, CoefficientsTable48000, (2.0f - value) * value);
}

/**
 * @brief Select coefficients for the 1024 Hz band-pass preset with 48 kHz compatibility.
 * @param pCoefficients Non-null destination for the five filter coefficients.
 * @param sampleRate Unused; this preset uses its fixed 48000 Hz coefficient table.
 * @param value Filter control value, normally between zero and one; the table index is clamped.
 */
void BiquadFilterBpf1024Nw4fCompatible48k::GetCoefficients(BiquadFilterCoefficients* pCoefficients,
                                                           int sampleRate, float value) const {
    ReadCoefficients(pCoefficients, CoefficientsTable48000, (2.0f - value) * value);
}

/**
 * @brief Select coefficients for the 2048 Hz band-pass preset with 48 kHz compatibility.
 * @param pCoefficients Non-null destination for the five filter coefficients.
 * @param sampleRate Unused; this preset uses its fixed 48000 Hz coefficient table.
 * @param value Filter control value, normally between zero and one; the table index is clamped.
 */
void BiquadFilterBpf2048Nw4fCompatible48k::GetCoefficients(BiquadFilterCoefficients* pCoefficients,
                                                           int sampleRate, float value) const {
    ReadCoefficients(pCoefficients, CoefficientsTable48000, (2.0f - value) * value);
}

} // namespace nn::atk::detail

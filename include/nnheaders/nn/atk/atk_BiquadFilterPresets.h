#pragma once
#include <nn/atk/atk_BiquadFilterCallback.h>

namespace nn::atk::detail {
class BiquadFilterLpf : public BiquadFilterCallback {
  public:
    /** @brief Destroy the low-pass preset without owning its static coefficient table. */
    ~BiquadFilterLpf() override = default;
    void GetCoefficients(BiquadFilterCoefficients* pCoefficients, int sampleRate, float value) const override;
    static const BiquadFilterCoefficients CoefficientsTable32000[112];
};

class BiquadFilterHpf : public BiquadFilterCallback {
  public:
    /** @brief Destroy the high-pass preset without owning its static coefficient table. */
    ~BiquadFilterHpf() override = default;
    void GetCoefficients(BiquadFilterCoefficients* pCoefficients, int sampleRate, float value) const override;
    static const BiquadFilterCoefficients CoefficientsTable32000[97];
};

class BiquadFilterBpf512 : public BiquadFilterCallback {
  public:
    /** @brief Destroy the 512 Hz band-pass preset without owning its static coefficient table. */
    ~BiquadFilterBpf512() override = default;
    void GetCoefficients(BiquadFilterCoefficients* pCoefficients, int sampleRate, float value) const override;
    static const BiquadFilterCoefficients CoefficientsTable32000[122];
};

class BiquadFilterBpf1024 : public BiquadFilterCallback {
  public:
    /** @brief Destroy the 1024 Hz band-pass preset without owning its static coefficient table. */
    ~BiquadFilterBpf1024() override = default;
    void GetCoefficients(BiquadFilterCoefficients* pCoefficients, int sampleRate, float value) const override;
    static const BiquadFilterCoefficients CoefficientsTable32000[93];
};

class BiquadFilterBpf2048 : public BiquadFilterCallback {
  public:
    /** @brief Destroy the 2048 Hz band-pass preset without owning its static coefficient table. */
    ~BiquadFilterBpf2048() override = default;
    void GetCoefficients(BiquadFilterCoefficients* pCoefficients, int sampleRate, float value) const override;
    static const BiquadFilterCoefficients CoefficientsTable32000[93];
};

class BiquadFilterLpfNw4fCompatible48k : public BiquadFilterCallback {
  public:
    /** @brief Destroy the low-pass preset without owning its static coefficient table. */
    ~BiquadFilterLpfNw4fCompatible48k() override = default;
    void GetCoefficients(BiquadFilterCoefficients* pCoefficients, int sampleRate, float value) const override;
    static const BiquadFilterCoefficients CoefficientsTable48000[112];
};

class BiquadFilterHpfNw4fCompatible48k : public BiquadFilterCallback {
  public:
    /** @brief Destroy the high-pass preset without owning its static coefficient table. */
    ~BiquadFilterHpfNw4fCompatible48k() override = default;
    void GetCoefficients(BiquadFilterCoefficients* pCoefficients, int sampleRate, float value) const override;
    static const BiquadFilterCoefficients CoefficientsTable48000[97];
};

class BiquadFilterBpf512Nw4fCompatible48k : public BiquadFilterCallback {
  public:
    /** @brief Destroy the 512 Hz band-pass preset without owning its static coefficient table. */
    ~BiquadFilterBpf512Nw4fCompatible48k() override = default;
    void GetCoefficients(BiquadFilterCoefficients* pCoefficients, int sampleRate, float value) const override;
    static const BiquadFilterCoefficients CoefficientsTable48000[122];
};

class BiquadFilterBpf1024Nw4fCompatible48k : public BiquadFilterCallback {
  public:
    /** @brief Destroy the 1024 Hz band-pass preset without owning its static coefficient table. */
    ~BiquadFilterBpf1024Nw4fCompatible48k() override = default;
    void GetCoefficients(BiquadFilterCoefficients* pCoefficients, int sampleRate, float value) const override;
    static const BiquadFilterCoefficients CoefficientsTable48000[93];
};

class BiquadFilterBpf2048Nw4fCompatible48k : public BiquadFilterCallback {
  public:
    /** @brief Destroy the 2048 Hz band-pass preset without owning its static coefficient table. */
    ~BiquadFilterBpf2048Nw4fCompatible48k() override = default;
    void GetCoefficients(BiquadFilterCoefficients* pCoefficients, int sampleRate, float value) const override;
    static const BiquadFilterCoefficients CoefficientsTable48000[93];
};

} // namespace nn::atk::detail

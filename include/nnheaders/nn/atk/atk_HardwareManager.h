#pragma once

#include <nn/atk/atk_Global.h>
#include <nn/audio.h>
#include <nn/os/os_Mutex.h>
#include <atomic>

namespace nn::atk::detail {
/**
 * @brief Interpolate a value over a counted transition.
 * @tparam ValueType Arithmetic type of the interpolated value.
 * @tparam CountType Arithmetic type of elapsed and total frame counts.
 */
template <typename ValueType, typename CountType> class MoveValue {
  public:
    /** @brief Test transition completion. @return Whether elapsed frames reached the duration. */
    bool IsFinished() const { return m_Counter >= m_Frame; }

    /** @brief Evaluate the transition at its current frame. @return Current value, or the target after
     * completion. */
    ValueType GetValue() const {
        if (IsFinished()) {
            return m_Target;
        }
        return static_cast<ValueType>(m_Origin + (m_Target - m_Origin) * m_Counter / m_Frame);
    }

  private:
    ValueType m_Origin;
    ValueType m_Target;
    CountType m_Frame;
    CountType m_Counter;
};
} // namespace nn::atk::detail

namespace nn::atk::detail::driver {
class HardwareManager : public Util::Singleton<HardwareManager> {
  public:
    class EffectAuxListScopedLock {
      public:
        EffectAuxListScopedLock();
        ~EffectAuxListScopedLock();
    };
    class EffectAuxListForFinalMixScopedLock {
      public:
        EffectAuxListForFinalMixScopedLock();
        ~EffectAuxListForFinalMixScopedLock();
    };
    class EffectAuxListForAdditionalSubMixScopedLock {
      public:
        EffectAuxListForAdditionalSubMixScopedLock();
        ~EffectAuxListForAdditionalSubMixScopedLock();
    };
    class SubMixListScopedLock {
      public:
        SubMixListScopedLock();
        ~SubMixListScopedLock();
    };
    struct HardwareManagerParameter {
        void SetSubMixParameter(bool enableStereoMode, bool enableEffect, bool enableSubMix,
                                bool enableAdditionalEffectBus, bool enableAdditionalSubMix,
                                bool enableCustomSubMix, int customSubMixCount, int customChannelCount);
        u8 _00[0x10];
        int subMixCount;
        int subMixTotalChannelCount;
        u8 _18;
        bool enableAdditionalEffectBus;
        bool enableAdditionalSubMix;
        u8 _1b[3];
        bool enableSubMix;
        u8 _1f[9];
        bool enableCustomSubMix;
    };
    void LockEffectAuxList();
    void UnlockEffectAuxList();
    void LockEffectAuxListForFinalMix();
    void UnlockEffectAuxListForFinalMix();
    void LockEffectAuxListForAdditionalSubMix();
    void UnlockEffectAuxListForAdditionalSubMix();
    void LockSubMixList();
    void UnlockSubMixList();
    void SetMasterVolume(f32 volume, int fadeFrames);
    void SetOutputMode(OutputMode mode, OutputDevice device);

    /** @brief Access the renderer configuration. @return Mutable configuration owned by this manager. */
    nn::audio::AudioRendererConfig& GetAudioRendererConfig() { return m_Config; }
    /** @brief Access renderer creation settings. @return Read-only renderer parameters. */
    const audio::AudioRendererParameter& GetAudioRendererParameter() const { return m_RendererParameter; }
    /** @brief Read the atomic renderer update counter. @return Completed renderer update count. */
    u64 GetAudioRendererUpdateCount() const { return m_RendererUpdateCount.load(); }
    /** @brief Acquire exclusive access to the audio renderer. */
    void LockAudioRenderer() { m_RendererMutex.Lock(); }
    /** @brief Release exclusive access to the audio renderer. */
    void UnlockAudioRenderer() { m_RendererMutex.Unlock(); }
    /** @brief Evaluate the current master-volume fade. @return Current master gain. */
    f32 GetMasterVolume() const { return m_MasterVolume.GetValue(); }
    /**
     * @brief Read the output mode of an audio device.
     * @param device Valid output device index, less than OutputDevice_Count.
     * @return Mode currently selected for the device.
     */
    OutputMode GetOutputMode(OutputDevice device) const { return m_OutputMode[device]; }

  private:
    u8 _0[0x18];
    nn::audio::AudioRendererConfig m_Config;
    u8 _70[0xc8 - 0x18 - sizeof(nn::audio::AudioRendererConfig)];
    std::atomic<u64> m_RendererUpdateCount;
    u8 _d0[0xe0 - 0xd0];
    OutputMode m_OutputMode[OutputDevice_Count];
    OutputMode m_EndUserOutputMode[OutputDevice_Count];
    u8 _e8[0xec - 0xe8];
    MoveValue<float, int> m_MasterVolume;
    u8 _fc[0x8a0 - 0xfc];
    os::Mutex m_SubMixListMutex;
    audio::AudioRendererParameter m_RendererParameter;
    u8 _8fc[0x960 - 0x8c0 - sizeof(audio::AudioRendererParameter)];
    os::Mutex m_RendererMutex;
    u8 _980[0x20];
    os::Mutex m_EffectAuxListMutex;
    os::Mutex m_EffectAuxListForFinalMixMutex;
    os::Mutex m_EffectAuxListForAdditionalSubMixMutex;
};
} // namespace nn::atk::detail::driver

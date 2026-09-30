#pragma once

#include <nn/atk/atk_Global.h>
#include <nn/audio.h>
#include <nn/os/os_Mutex.h>
#include <atomic>

namespace nn::atk::detail {
template <typename ValueType, typename CountType>
class MoveValue {
public:
    bool IsFinished() const { return m_Counter >= m_Frame; }

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
}  // namespace nn::atk::detail

namespace nn::atk::detail::driver {
class HardwareManager : public Util::Singleton<HardwareManager> {
public:
    void SetMasterVolume(f32 volume, int fadeFrames);
    void SetOutputMode(OutputMode mode, OutputDevice device);

    nn::audio::AudioRendererConfig& GetAudioRendererConfig() { return m_Config; }
    const audio::AudioRendererParameter& GetAudioRendererParameter() const { return m_RendererParameter; }
    u64 GetAudioRendererUpdateCount() const { return m_RendererUpdateCount.load(); }
    void LockAudioRenderer() { m_RendererMutex.Lock(); }
    void UnlockAudioRenderer() { m_RendererMutex.Unlock(); }
    f32 GetMasterVolume() const { return m_MasterVolume.GetValue(); }
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
    u8 _fc[0x8c0 - 0xfc];
    audio::AudioRendererParameter m_RendererParameter;
    u8 _8fc[0x960 - 0x8c0 - sizeof(audio::AudioRendererParameter)];
    os::Mutex m_RendererMutex;
};
}  // namespace nn::atk::detail::driver

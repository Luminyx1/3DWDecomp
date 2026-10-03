#pragma once

#include <nn/atk/atk_SoundHandle.h>

namespace nn::atk {
namespace detail {
class WaveSound : public BasicSound {
public:
    s64 GetPlaySamplePosition(bool isOriginalSamplePosition) const;
};
}  // namespace detail

class WaveSoundHandle {
public:
    explicit WaveSoundHandle(SoundHandle* pHandle);
    ~WaveSoundHandle() { DetachSound(); }

    void DetachSound();
    bool IsAttachedSound() const { return m_pSound != nullptr; }
    bool IsPrepared() const { return IsAttachedSound() && m_pSound->IsPrepared(); }

    s64 GetPlaySamplePosition() const {
        if (!IsAttachedSound()) {
            return -1;
        }
        return m_pSound->GetPlaySamplePosition(true);
    }

    void SetMixMode(MixMode mode) {
        if (IsAttachedSound()) {
            m_pSound->SetMixMode(mode);
        }
    }

    void SetOutputChannelMixParameter(OutputDevice device, u32 channel, const MixParameter& rParam) {
        if (IsAttachedSound()) {
            m_pSound->SetOutputChannelMixParameter(device, channel, rParam);
        }
    }

private:
    detail::WaveSound* m_pSound;
};
}  // namespace nn::atk

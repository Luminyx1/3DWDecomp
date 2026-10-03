#pragma once

#include <nn/atk/atk_BasicSound.h>

namespace nn::atk {
class SoundHandle {
public:
    SoundHandle() : m_pSound(nullptr) {}
    ~SoundHandle() { DetachSound(); }

    bool IsAttachedSound() const { return m_pSound != nullptr; }

    void DetachSound();
    void detail_DuplicateHandle(SoundHandle* handle);
    void detail_AttachSoundAsTempHandle(detail::BasicSound* sound);
    void detail_AttachSound(detail::BasicSound* sound);
    bool CalculateSoundParamCalculationValues(SoundParamCalculationValues* values) const;
    detail::BasicSound* detail_GetAttachedSound() { return m_pSound; }
    const detail::BasicSound* detail_GetAttachedSound() const { return m_pSound; }

    bool IsPause() const {
        if (IsAttachedSound() && m_pSound->IsPause()) {
            return true;
        }
        return false;
    }

    f32 GetVolume() const {
        if (IsAttachedSound()) {
            return m_pSound->GetVolume();
        }
        return 0.0f;
    }

    void SetSurroundPan(f32 pan) {
        if (IsAttachedSound()) {
            m_pSound->SetSurroundPan(pan);
        }
    }

    void SetMainSend(f32 send) {
        if (IsAttachedSound()) {
            m_pSound->SetMainSend(send);
        }
    }

    void SetFxSend(AuxBus bus, f32 send) {
        if (IsAttachedSound()) {
            m_pSound->SetFxSend(bus, send);
        }
    }

    void StartPrepared() {
        if (IsAttachedSound()) {
            m_pSound->StartPrepared();
        }
    }

    bool IsPrepared() const {
        if (IsAttachedSound() && m_pSound->IsPrepared()) {
            return true;
        }
        return false;
    }

    void FadeIn(int frames) {
        if (IsAttachedSound()) {
            m_pSound->FadeIn(frames);
        }
    }

    void SetOutputLine(u32 lineFlag) {
        if (IsAttachedSound()) {
            m_pSound->SetOutputLine(lineFlag);
        }
    }

    void SetOutputEffectSend(OutputDevice device, AuxBus bus, f32 send) {
        if (IsAttachedSound()) {
            m_pSound->SetOutputFxSend(device, bus, send);
        }
    }

    void SetLpfFreq(f32 freq) {
        if (IsAttachedSound()) {
            m_pSound->SetLpfFreq(freq);
        }
    }

    void SetBiquadFilter(int type, f32 value) {
        if (IsAttachedSound()) {
            m_pSound->SetBiquadFilter(type, value);
        }
    }

    void Stop(int fadeFrames) {
        if (IsAttachedSound()) {
            m_pSound->Stop(fadeFrames);
        }
    }

    void Pause(bool flag, int fadeFrames) {
        if (IsAttachedSound()) {
            m_pSound->Pause(flag, fadeFrames);
        }
    }

    void SetVolume(f32 volume, int frames) {
        if (IsAttachedSound()) {
            m_pSound->SetVolume(volume, frames);
        }
    }

    void SetPitch(f32 pitch) {
        if (IsAttachedSound()) {
            m_pSound->SetPitch(pitch);
        }
    }

    void SetPan(f32 pan) {
        if (IsAttachedSound()) {
            m_pSound->SetPan(pan);
        }
    }

    u32 GetId() const {
        if (IsAttachedSound()) {
            return m_pSound->GetId();
        }
        return 0xffffffff;
    }

private:
    friend class StreamSoundHandle;
    friend class SequenceSoundHandle;
    detail::BasicSound* m_pSound;
};
}  // namespace nn::atk

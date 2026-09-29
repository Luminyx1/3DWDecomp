#pragma once

#include <nn/atk/atk_BasicSound.h>

namespace nn::atk {
class SoundHandle {
public:
    SoundHandle() : m_pSound(nullptr) {}
    ~SoundHandle() { DetachSound(); }

    bool IsAttachedSound() const { return m_pSound != nullptr; }

    void DetachSound();

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
    detail::BasicSound* m_pSound;
};
}  // namespace nn::atk

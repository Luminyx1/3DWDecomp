#pragma once

#include <nn/atk/atk_SoundHandle.h>

namespace nn::atk {
namespace detail {
class SequenceSound : public BasicSound {
public:
    static bool ReadGlobalVariable(int index, s16* pVar);
    static bool WriteGlobalVariable(int index, s16 value);
    bool ReadVariable(int index, s16* pVar) const;
    void WriteVariable(int index, s16 value);
    bool ReadTrackVariable(int track, int index, s16* pVar) const;
    void WriteTrackVariable(int track, int index, s16 value);
    void SetTempoRatio(f32 ratio);
};
}  // namespace detail

class SequenceSoundHandle {
public:
    explicit SequenceSoundHandle(SoundHandle* pHandle);
    ~SequenceSoundHandle() { DetachSound(); }

    void DetachSound();
    bool IsAttachedSound() const { return m_pSound != nullptr; }

    static bool ReadGlobalVariable(int index, s16* pVar) {
        return detail::SequenceSound::ReadGlobalVariable(index, pVar);
    }

    static bool WriteGlobalVariable(int index, s16 value) {
        return detail::SequenceSound::WriteGlobalVariable(index, value);
    }

    bool ReadVariable(int index, s16* pVar) const {
        if (!IsAttachedSound()) {
            return false;
        }
        return m_pSound->ReadVariable(index, pVar);
    }

    bool WriteVariable(int index, s16 value) {
        if (!IsAttachedSound()) {
            return false;
        }
        m_pSound->WriteVariable(index, value);
        return true;
    }

    bool ReadTrackVariable(int track, int index, s16* pVar) const {
        if (!IsAttachedSound()) {
            return false;
        }
        return m_pSound->ReadTrackVariable(track, index, pVar);
    }

    bool WriteTrackVariable(int track, int index, s16 value) {
        if (!IsAttachedSound()) {
            return false;
        }
        m_pSound->WriteTrackVariable(track, index, value);
        return true;
    }

    void SetTempoRatio(f32 ratio) {
        if (IsAttachedSound()) {
            m_pSound->SetTempoRatio(ratio);
        }
    }

private:
    detail::SequenceSound* m_pSound;
};
}  // namespace nn::atk

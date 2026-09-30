#include <nn/atk/atk_SoundHandle.h>

namespace nn::atk {
// handle supplies the sound to reference temporarily; null leaves this handle detached.
void SoundHandle::detail_DuplicateHandle(SoundHandle* handle) {
    DetachSound();
    if (handle && handle->IsAttachedSound()) detail_AttachSoundAsTempHandle(handle->m_pSound);
}

void SoundHandle::DetachSound() {
    if (IsAttachedSound()) {
        if (m_pSound->mGeneralHandle == this) m_pSound->mGeneralHandle = nullptr;
        if (m_pSound->mTempGeneralHandle == this) m_pSound->mTempGeneralHandle = nullptr;
        if (IsAttachedSound()) m_pSound = nullptr;
    }
}

// sound becomes the temporary attachment, replacing its previous temporary handle.
void SoundHandle::detail_AttachSoundAsTempHandle(detail::BasicSound* sound) {
    m_pSound = sound;
    if (m_pSound->IsAttachedTempGeneralHandle()) m_pSound->DetachTempGeneralHandle();
    m_pSound->mTempGeneralHandle = this;
}

// sound becomes the primary attachment, replacing its previous general handle.
void SoundHandle::detail_AttachSound(detail::BasicSound* sound) {
    m_pSound = sound;
    if (m_pSound->IsAttachedGeneralHandle()) m_pSound->DetachGeneralHandle();
    m_pSound->mGeneralHandle = this;
}

// values receives the attached sound's calculated parameters. False means no sound is attached.
bool SoundHandle::CalculateSoundParamCalculationValues(SoundParamCalculationValues* values) const {
    if (!IsAttachedSound()) return false;
    m_pSound->CalculateSoundParamCalculationValues(values);
    return true;
}
}

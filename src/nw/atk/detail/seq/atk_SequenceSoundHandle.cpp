#include <nn/atk/atk_SpecialSoundHandle.h>

namespace nn::atk {
// handle supplies a sequence sound; null, detached, or incompatible handles leave this detached.
SequenceSoundHandle::SequenceSoundHandle(SoundHandle* handle) : mSound(nullptr) {
    if (handle && handle->IsAttachedSound()) {
        auto* sound = detail::SoundCast<detail::SequenceSound>(handle->m_pSound);
        if (sound) detail_AttachSoundAsTempHandle(sound);
    }
}
// sound receives this temporary handle after detaching its previous special handle.
void SequenceSoundHandle::detail_AttachSoundAsTempHandle(detail::SequenceSound* sound) {
    mSound = sound;
    if (mSound->IsAttachedTempSpecialHandle()) mSound->DetachTempSpecialHandle();
    mSound->mTempHandle = this;
}
void SequenceSoundHandle::DetachSound() {
    if (mSound) {
        if (mSound->mTempHandle == this) mSound->mTempHandle = nullptr;
        if (mSound) mSound = nullptr;
    }
}
}

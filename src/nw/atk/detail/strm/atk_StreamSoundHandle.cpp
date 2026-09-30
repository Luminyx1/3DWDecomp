#include <nn/atk/atk_SpecialSoundHandle.h>

namespace nn::atk {
// handle supplies a stream sound; null, detached, or incompatible handles leave this detached.
StreamSoundHandle::StreamSoundHandle(SoundHandle* handle) : mSound(nullptr) {
    if (handle && handle->IsAttachedSound()) {
        auto* sound = detail::SoundCast<detail::StreamSound>(handle->m_pSound);

        if (sound) detail_AttachSoundAsTempHandle(sound);
    }
}

// sound receives this temporary handle after detaching its previous special handle.
void StreamSoundHandle::detail_AttachSoundAsTempHandle(detail::StreamSound* sound) {
    mSound = sound;

    if (mSound->IsAttachedTempSpecialHandle()) mSound->DetachTempSpecialHandle();
    mSound->mTempHandle = this;
}

void StreamSoundHandle::DetachSound() {
    if (mSound) {
        if (mSound->mTempHandle == this) mSound->mTempHandle = nullptr;

        if (mSound) mSound = nullptr;
    }
}
}

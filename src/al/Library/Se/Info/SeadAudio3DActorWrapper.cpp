#include "Library/Se/Info/SeadAudioActorWrapper.hpp"

#include <audio/seadSoundHandle.h>
#include <nn/util/util_VectorApi.h>

namespace al {
/**
 * Starts a sound with start information.
 * @param pHandle Sound handle.
 * @param soundId Sound id.
 * @param pStartInfo Start information.
 * @param isHold Unused.
 * @return True if the sound started.
 */
bool SeadAudio3DActorWrapper::startSoundWithInfo(sead::SoundHandle* pHandle, u32 soundId,
                                                 const SoundStartInfo* pStartInfo, bool isHold) {
    return StartSound(pHandle, soundId, reinterpret_cast<const nn::atk::SoundStartable::StartInfo*>(pStartInfo))
        .IsSuccess();
}

/**
 * Gets the actor position.
 * @return Actor position.
 */
const sead::Vector3f* SeadAudio3DActorWrapper::getPosition() const {
    return reinterpret_cast<const sead::Vector3f*>(&GetPosition());
}

/**
 * Resets the actor velocity.
 */
void SeadAudio3DActorWrapper::resetVelocity() {
    nn::util::Vector3fType zero;
    nn::util::VectorSet(&zero, 0.0f, 0.0f, 0.0f);
    SetVelocity(zero);
}

/**
 * Checks whether the actor plays any sound.
 * @return True if the actor plays a sound.
 */
bool SeadAudio3DActorWrapper::isPlayingSound() const {
    for (s32 i = 0; i < 4; i++) {
        if (GetPlayingSoundCount(i) > 0) {
            return true;
        }
    }
    return false;
}
}  // namespace al

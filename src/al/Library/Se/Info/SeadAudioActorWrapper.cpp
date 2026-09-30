#include "Library/Se/Info/SeadAudioActorWrapper.hpp"

#include <audio/seadSoundHandle.h>

#include "Project/Audio/System/AudioPlayer.hpp"

namespace al {
/**
 * Stops all sounds of the actor.
 */
SeadAudioActorWrapper::~SeadAudioActorWrapper() {
    stopAllSound(0);
}

/**
 * Stops all sounds of the actor.
 * @param fadeFrames Fade out frames.
 */
void SeadAudioActorWrapper::stopAllSound(s32 fadeFrames) {
    StopAllSound(fadeFrames);
}

/**
 * Initializes the actor with the sound archive player of an audio player.
 * @param pPlayer Audio player.
 */
void SeadAudioActorWrapper::initialize(SeadAudioPlayer* pPlayer) {
    Initialize(pPlayer);
}

/**
 * Starts a sound with start information.
 * @param pHandle Sound handle.
 * @param soundId Sound id.
 * @param pStartInfo Start information.
 * @param isHold Unused.
 * @return True if the sound started.
 */
bool SeadAudioActorWrapper::startSoundWithInfo(sead::SoundHandle* pHandle, u32 soundId,
                                               const SoundStartInfo* pStartInfo, bool isHold) {
    return StartSound(pHandle, soundId, reinterpret_cast<const nn::atk::SoundStartable::StartInfo*>(pStartInfo))
        .IsSuccess();
}

/**
 * Checks whether the actor plays any sound.
 * @return True if a sound is playing.
 */
bool SeadAudioActorWrapper::isPlayingSound() const {
    for (s32 i = 0; i < 4; i++) {
        if (GetPlayingSoundCount(i) > 0) {
            return true;
        }
    }
    return false;
}
}  // namespace al

#include "Project/Audio/Sound/SoundHandle.hpp"

namespace al {
/**
 * @brief Constructs a sound handle not attached to any sound.
 */
AcLSoundHandle::AcLSoundHandle() = default;

/**
 * @brief Detaches the handle from the sound it controls, leaving the sound playing.
 */
void AcLSoundHandle::detachSound() {
    DetachSound();
}

/**
 * @brief Stops the attached sound.
 * @param fadeFrames The number of frames to fade out over.
 */
void AcLSoundHandle::stop(s32 fadeFrames) {
    sead::SoundHandle::stop(fadeFrames);
}

/**
 * @brief Pauses the attached sound.
 * @param fadeFrames The number of frames to fade out over.
 */
void AcLSoundHandle::pause(s32 fadeFrames) {
    sead::SoundHandle::pause(fadeFrames);
}

/**
 * @brief Resumes the attached sound after a pause.
 * @param fadeFrames The number of frames to fade in over.
 */
void AcLSoundHandle::unpause(s32 fadeFrames) {
    sead::SoundHandle::unpause(fadeFrames);
}

/**
 * @brief Changes the volume of the attached sound.
 * @param volume The target volume.
 * @param frames The number of frames to reach the target volume over.
 */
void AcLSoundHandle::setVolume(f32 volume, s32 frames) {
    sead::SoundHandle::setVolume(volume, frames);
}

/**
 * @brief Checks whether the handle currently controls a sound.
 * @return Whether a sound is attached.
 */
bool AcLSoundHandle::isAttachedSound() const {
    return sead::SoundHandle::isAttachedSound();
}

/**
 * @brief Destroys the handle, detaching it from its sound.
 */
AcLSoundHandle::~AcLSoundHandle() = default;
}  // namespace al

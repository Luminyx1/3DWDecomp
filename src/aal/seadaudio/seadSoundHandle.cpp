#include "audio/seadSoundHandle.h"

namespace sead {
/**
 * Stops the attached sound.
 * @param fadeFrames Fade-out length in frames.
 */
void SoundHandle::stop(s32 fadeFrames) {
    Stop(fadeFrames);
}

/**
 * Pauses the attached sound.
 * @param fadeFrames Fade-out length in frames.
 */
void SoundHandle::pause(s32 fadeFrames) {
    Pause(true, fadeFrames);
}

/**
 * Resumes the attached sound.
 * @param fadeFrames Fade-in length in frames.
 */
void SoundHandle::unpause(s32 fadeFrames) {
    Pause(false, fadeFrames);
}

/**
 * Sets the volume of the attached sound.
 * @param volume Volume ratio.
 * @param frames Length of the volume change in frames.
 */
void SoundHandle::setVolume(f32 volume, s32 frames) {
    SetVolume(volume, frames);
}

/**
 * Sets the pitch of the attached sound.
 * @param pitch Pitch ratio.
 */
void SoundHandle::setPitch(f32 pitch) {
    SetPitch(pitch);
}

/**
 * Sets the pan of the attached sound.
 * @param pan Pan position.
 */
void SoundHandle::setPan(f32 pan) {
    SetPan(pan);
}

/**
 * Checks whether a sound is attached to the handle.
 * @return True if a sound is attached.
 */
bool SoundHandle::isAttachedSound() const {
    return IsAttachedSound();
}

/**
 * Gets the ID of the attached sound.
 * @return Sound ID, or 0xffffffff if no sound is attached.
 */
u32 SoundHandle::getSoundId() const {
    return GetId();
}
}  // namespace sead

#include "Project/Audio/Sound/SoundHandle.hpp"

#include <nn/atk/atk_SequenceSoundHandle.h>
#include <nn/atk/atk_WaveSoundHandle.h>

namespace al {
/**
 * Constructs a platform handle with no sound attached.
 */
AcLSoundHandlePlatform::AcLSoundHandlePlatform() = default;

/**
 * Detaches the sound from the handle.
 */
void AcLSoundHandlePlatform::detachSound() {
    DetachSound();
}

/**
 * Checks whether the attached sound is paused.
 * @return True if a sound is attached and paused.
 */
bool AcLSoundHandlePlatform::isPause() const {
    return IsPause();
}

/**
 * Gets the volume of the attached sound.
 * @return Volume, or 0 if no sound is attached.
 */
f32 AcLSoundHandlePlatform::getVolume() const {
    return GetVolume();
}

/**
 * Sets the surround pan of the attached sound.
 * @param pan Surround pan.
 */
void AcLSoundHandlePlatform::setSurroundPan(f32 pan) {
    SetSurroundPan(pan);
}

/**
 * Sets the main send of the attached sound.
 * @param send Main send.
 */
void AcLSoundHandlePlatform::setMainSend(f32 send) {
    SetMainSend(send);
}

/**
 * Sets an effect send of the attached sound.
 * @param bus Aux bus index.
 * @param send Send level.
 */
void AcLSoundHandlePlatform::setFxSend(s32 bus, f32 send) {
    SetFxSend(static_cast<nn::atk::AuxBus>(bus), send);
}

/**
 * Sets the low-pass filter frequency of the attached sound.
 * @param freq Filter frequency.
 */
void AcLSoundHandlePlatform::setLpfFreq(f32 freq) {
    SetLpfFreq(freq);
}

/**
 * Sets the biquad filter of the attached sound.
 * @param type Filter type.
 * @param value Filter value.
 */
void AcLSoundHandlePlatform::setBiquadFilter(s32 type, f32 value) {
    SetBiquadFilter(type, value);
}

/**
 * Reads a sequence global variable.
 * @param index Variable index.
 * @param pVar Receives the value.
 * @return True on success.
 */
bool AcLSoundHandlePlatform::readSeqGlobalVariable(s32 index, s16* pVar) {
    return nn::atk::SequenceSoundHandle::ReadGlobalVariable(index, pVar);
}

/**
 * Writes a sequence global variable.
 * @param index Variable index.
 * @param value Value to write.
 * @return Always true.
 */
bool AcLSoundHandlePlatform::writeSeqGlobalVariable(s32 index, s16 value) {
    nn::atk::SequenceSoundHandle::WriteGlobalVariable(index, value);
    return true;
}

/**
 * Reads a local variable of the attached sequence sound.
 * @param index Variable index.
 * @param pVar Receives the value.
 * @return True on success.
 */
bool AcLSoundHandlePlatform::readSeqLocalVariable(s32 index, s16* pVar) {
    nn::atk::SequenceSoundHandle handle(this);
    return handle.ReadVariable(index, pVar);
}

/**
 * Writes a local variable of the attached sequence sound.
 * @param index Variable index.
 * @param value Value to write.
 * @return True if a sequence sound is attached.
 */
bool AcLSoundHandlePlatform::writeSeqLocalVariable(s32 index, s16 value) {
    nn::atk::SequenceSoundHandle handle(this);
    return handle.WriteVariable(index, value);
}

/**
 * Reads a track variable of the attached sequence sound.
 * @param track Track index.
 * @param index Variable index.
 * @param pVar Receives the value.
 * @return True on success.
 */
bool AcLSoundHandlePlatform::readSeqTrackVariable(s32 track, s32 index, s16* pVar) {
    nn::atk::SequenceSoundHandle handle(this);
    return handle.ReadTrackVariable(track, index, pVar);
}

/**
 * Writes a track variable of the attached sequence sound.
 * @param track Track index.
 * @param index Variable index.
 * @param value Value to write.
 * @return True if a sequence sound is attached.
 */
bool AcLSoundHandlePlatform::writeSeqTrackVariable(s32 track, s32 index, s16 value) {
    nn::atk::SequenceSoundHandle handle(this);
    return handle.WriteTrackVariable(track, index, value);
}

/**
 * Sets the tempo ratio of the attached sequence sound.
 * @param ratio Tempo ratio.
 */
void AcLSoundHandlePlatform::setSeqTempoRatio(f32 ratio) {
    nn::atk::SequenceSoundHandle handle(this);
    handle.SetTempoRatio(ratio);
}

/**
 * Sets the per-speaker volumes of the attached wave sound for one output device.
 * @param device Output device.
 * @param frontLeft Front left volume.
 * @param frontRight Front right volume.
 * @param rearLeft Rear left volume.
 * @param rearRight Rear right volume.
 * @param frontCenter Front center volume.
 * @param lfe LFE volume.
 */
void AcLSoundHandlePlatform::setOutputDeviceSpeakerVolume(s32 device, f32 frontLeft, f32 frontRight,
                                                          f32 rearLeft, f32 rearRight,
                                                          f32 frontCenter, f32 lfe) {
    nn::atk::WaveSoundHandle handle(this);
    handle.SetMixMode(nn::atk::MixMode_MixParameter);
    nn::atk::MixParameter param = {{frontLeft, frontRight, rearLeft, rearRight, frontCenter, lfe}};
    handle.SetOutputChannelMixParameter(static_cast<nn::atk::OutputDevice>(device), 0, param);
}

/**
 * Sets the per-speaker volumes of the attached wave sound for all output devices.
 * @param frontLeft Front left volume.
 * @param frontRight Front right volume.
 * @param rearLeft Rear left volume.
 * @param rearRight Rear right volume.
 * @param frontCenter Front center volume.
 * @param lfe LFE volume.
 */
void AcLSoundHandlePlatform::setAllOutputDeviceSpeakerVolume(f32 frontLeft, f32 frontRight,
                                                             f32 rearLeft, f32 rearRight,
                                                             f32 frontCenter, f32 lfe) {
    nn::atk::WaveSoundHandle handle(this);
    handle.SetMixMode(nn::atk::MixMode_MixParameter);
    nn::atk::MixParameter param = {{frontLeft, frontRight, rearLeft, rearRight, frontCenter, lfe}};
    for (s32 i = 0; i < nn::atk::OutputDevice_Count; i++) {
        handle.SetOutputChannelMixParameter(static_cast<nn::atk::OutputDevice>(i), 0, param);
    }
}

/**
 * Constructs a handle with no sound attached.
 */
AcLSoundHandle::AcLSoundHandle() = default;

/**
 * Detaches the sound from the handle.
 */
void AcLSoundHandle::detachSound() {
    DetachSound();
}

/**
 * Stops the attached sound.
 * @param fadeFrames Fade-out length in frames.
 */
void AcLSoundHandle::stop(s32 fadeFrames) {
    sead::SoundHandle::stop(fadeFrames);
}

/**
 * Pauses the attached sound.
 * @param fadeFrames Fade-out length in frames.
 */
void AcLSoundHandle::pause(s32 fadeFrames) {
    sead::SoundHandle::pause(fadeFrames);
}

/**
 * Resumes the attached sound.
 * @param fadeFrames Fade-in length in frames.
 */
void AcLSoundHandle::unpause(s32 fadeFrames) {
    sead::SoundHandle::unpause(fadeFrames);
}

/**
 * Sets the volume of the attached sound.
 * @param volume Volume ratio.
 * @param fadeFrames Length of the volume change in frames.
 */
void AcLSoundHandle::setVolume(f32 volume, s32 fadeFrames) {
    sead::SoundHandle::setVolume(volume, fadeFrames);
}
}  // namespace al

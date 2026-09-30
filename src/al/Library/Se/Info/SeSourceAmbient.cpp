#include "Library/Se/Info/SeSource.hpp"

#include "Library/Audio/System/AudioSystemInfo.hpp"
#include "Library/Se/Info/SeadAudioActorWrapper.hpp"
#include "Project/Audio/Sound/SoundHandle.hpp"
#include "Project/Audio/System/AudioConstMultiPlatformCommon.hpp"

namespace {
const sead::Vector3f cAmbientPos(0.0f, 0.0f, 0.0f);
}

namespace al {
/**
 * Constructs an ambient source and its actor.
 * @param pInfo Audio system information.
 */
SeSourceAmbient::SeSourceAmbient(AudioSystemInfo* pInfo) : SeSource("環境音源"), mInfo(pInfo) {
    mActor = new SeadAudioActorWrapper();
    mActor->initialize(mInfo->getSeadAudioPlayerForSe());
}

/**
 * Starts a sound on the actor.
 * @param pHandle Sound handle.
 * @param soundId Sound id.
 * @param pStartInfo Start information.
 * @return True if the sound started.
 */
bool SeSourceAmbient::startSound(AcLSoundHandle* pHandle, u32 soundId, const SoundStartInfo* pStartInfo) {
    return mActor->startSoundWithInfo(pHandle, soundId, pStartInfo, false);
}

/**
 * Checks whether the actor plays any sound.
 * @return True if a sound is playing.
 */
bool SeSourceAmbient::isPlayingSound() const {
    return mActor->isPlayingSound();
}

/**
 * Gets the source priority.
 * @return Maximum priority.
 */
s32 SeSourceAmbient::getPriority() const {
    return private_class::AudioConstMultiPlatformCommon::SOUND_REQUEST_PRIORITY_MAX;
}

/**
 * Gets the source position.
 * @return Origin.
 */
const sead::Vector3f* SeSourceAmbient::getPosition() const {
    return &cAmbientPos;
}

/**
 * Does nothing.
 */
void SeSourceAmbient::init() {}

/**
 * Does nothing.
 */
void SeSourceAmbient::update() {}

/**
 * Does nothing.
 */
void SeSourceAmbient::resetVelocity() {}
}  // namespace al

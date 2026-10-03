#include "Library/Se/Info/SeSource.hpp"

#include "Library/Audio/System/AudioSystemInfo.hpp"
#include "Library/Se/Info/SeadAudioActorWrapper.hpp"
#include "Project/Audio/Sound/SoundHandle.hpp"
#include "Project/Audio/System/AudioConstMultiPlatformCommon.hpp"

#include <audio/seadSoundHandle.h>
#include <attributes.h>
#include "Project/Audio/System/AudioPlayer.hpp"

namespace {
const sead::Vector3f cAmbientPos(0.0f, 0.0f, 0.0f);
}

namespace al {
/**
 * @brief Constructs an ambient source and its actor.
 * @param pInfo Non-null audio system information, retained for the source lifetime.
 */
SeSourceAmbient::SeSourceAmbient(AudioSystemInfo* pInfo) : SeSource("環境音源"), mInfo(pInfo) {
    mActor = new SeadAudioActorWrapper();
    mActor->initialize(mInfo->getSeadAudioPlayerForSe());
}

/**
 * @brief Starts a sound on the actor.
 * @param pHandle Non-null handle receiving the started sound.
 * @param soundId Sound id.
 * @param pStartInfo Optional sound start settings.
 * @return True if the sound started.
 */
bool SeSourceAmbient::startSound(AcLSoundHandle* pHandle, u32 soundId, const SoundStartInfo* pStartInfo) {
    return mActor->startSoundWithInfo(pHandle, soundId, pStartInfo, false);
}

/**
 * @brief Checks whether the actor plays any sound.
 * @return True if a sound is playing.
 */
bool SeSourceAmbient::isPlayingSound() const { return mActor->isPlayingSound(); }

/**
 * @brief Gets the source priority.
 * @return Maximum priority.
 */
s32 SeSourceAmbient::getPriority() const {
    return private_class::AudioConstMultiPlatformCommon::SOUND_REQUEST_PRIORITY_MAX;
}

/**
 * @brief Gets the source position.
 * @return Origin.
 */
const sead::Vector3f* SeSourceAmbient::getPosition() const { return &cAmbientPos; }

/**
 * @brief Does nothing.
 */
void SeSourceAmbient::init() {}

/**
 * @brief Does nothing.
 */
void SeSourceAmbient::update() {}

/**
 * @brief Does nothing.
 */
void SeSourceAmbient::resetVelocity() {}

/**
 * @brief Stops all sounds of the actor.
 */
SeadAudioActorWrapper::~SeadAudioActorWrapper() { stopAllSound(0); }

/**
 * @brief Stops all sounds of the actor.
 * @param fadeFrames Fade duration in frames; zero stops immediately.
 */
void SeadAudioActorWrapper::stopAllSound(s32 fadeFrames) { StopAllSound(fadeFrames); }

/**
 * @brief Initializes the actor with the sound archive player of an audio player.
 * @param pPlayer Non-null audio player supplying the sound archive player.
 */
NOINLINE void SeadAudioActorWrapper::initialize(SeadAudioPlayer* pPlayer) { Initialize(pPlayer); }

/**
 * @brief Starts a sound with start information.
 * @param pHandle Non-null handle receiving the started sound.
 * @param soundId Sound id.
 * @param pStartInfo Optional sound start settings.
 * @param isHold Unused.
 * @return True if the sound started.
 */
NOINLINE bool SeadAudioActorWrapper::startSoundWithInfo(sead::SoundHandle* pHandle, u32 soundId,
                                                        const SoundStartInfo* pStartInfo, bool isHold) {
    return StartSound(pHandle, soundId,
                      reinterpret_cast<const nn::atk::SoundStartable::StartInfo*>(pStartInfo))
        .IsSuccess();
}

/**
 * @brief Checks whether the actor plays any sound.
 * @return True if a sound is playing.
 */
NOINLINE bool SeadAudioActorWrapper::isPlayingSound() const {
    for (s32 i = 0; i < 4; i++) {
        if (GetPlayingSoundCount(i) > 0) {
            return true;
        }
    }

    return false;
}
} // namespace al

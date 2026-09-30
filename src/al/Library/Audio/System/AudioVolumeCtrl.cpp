#include "Library/Audio/System/AudioVolumeCtrl.hpp"

#include "Library/Audio/AudioMic.hpp"
#include "Library/Audio/System/AudioKeeper.hpp"
#include "Project/Audio/IUseAudioKeeper.hpp"

namespace {
bool isEnableMic(const al::IUseAudioKeeper* pUser) {
    return pUser->getAudioKeeper() != nullptr && pUser->getAudioKeeper()->getAudioMic() != nullptr &&
           !pUser->getAudioKeeper()->isForceInvalidSe();
}
}  // namespace

namespace al {
/**
 * Constructs the volume controller.
 */
AudioVolumeCtrl::AudioVolumeCtrl() = default;

/**
 * Does nothing.
 */
void AudioVolumeCtrl::init() {}

/**
 * Does nothing.
 */
void AudioVolumeCtrl::update() {}

/**
 * Gets the microphone input power.
 * @param pUser Audio user.
 * @return Input power, or 0 if the microphone is unavailable.
 */
f32 getMicInputPowerOld(const IUseAudioKeeper* pUser) {
    if (!isEnableMic(pUser)) {
        return 0.0f;
    }
    return pUser->getAudioKeeper()->getAudioMic()->getMicInputPower();
}

/**
 * Gets the microphone input power ratio.
 * @param pUser Audio user.
 * @return Input power ratio, or 0 if the microphone is unavailable.
 */
f32 getMicInputPowerRatio(const IUseAudioKeeper* pUser) {
    if (!isEnableMic(pUser)) {
        return 0.0f;
    }
    return pUser->getAudioKeeper()->getAudioMic()->getMicInputPowerRatio();
}

/**
 * Checks whether the microphone has input.
 * @param pUser Audio user.
 * @return True if there is input.
 */
bool isMicInputOn(const IUseAudioKeeper* pUser) {
    if (!isEnableMic(pUser)) {
        return false;
    }
    return pUser->getAudioKeeper()->getAudioMic()->isMicInput();
}

/**
 * Gets the breath power detected by the microphone.
 * @param pUser Audio user.
 * @return Breath power, or 0 if the microphone is unavailable.
 */
f32 getMicBreathPowerOld(const IUseAudioKeeper* pUser) {
    if (!isEnableMic(pUser)) {
        return 0.0f;
    }
    return pUser->getAudioKeeper()->getAudioMic()->getBreathPower();
}

/**
 * Gets the breath power ratio detected by the microphone.
 * @param pUser Audio user.
 * @return Breath power ratio, or 0 if the microphone is unavailable.
 */
f32 getMicBreathPowerRatio(const IUseAudioKeeper* pUser) {
    if (!isEnableMic(pUser)) {
        return 0.0f;
    }
    return pUser->getAudioKeeper()->getAudioMic()->getBreathPowerRatio();
}

/**
 * Checks whether the microphone detects breath.
 * @param pUser Audio user.
 * @return True if breath is detected.
 */
bool isMicBreathInputOn(const IUseAudioKeeper* pUser) {
    if (!isEnableMic(pUser)) {
        return false;
    }
    return pUser->getAudioKeeper()->getAudioMic()->isBreathInput();
}

/**
 * Starts microphone sampling.
 * @param pUser Audio user.
 */
void startMicSampling(const IUseAudioKeeper* pUser) {
    if (pUser->getAudioKeeper()->isForceInvalidSe()) {
        return;
    }
    AudioMic* mic = pUser->getAudioKeeper()->getAudioMic();
    if (mic != nullptr) {
        mic->startSampling();
    }
}

/**
 * Starts microphone sampling regardless of the input state.
 * @param pUser Audio user.
 */
void startMicSamplingForce(const IUseAudioKeeper* pUser) {
    if (pUser->getAudioKeeper()->isForceInvalidSe()) {
        return;
    }
    AudioMic* mic = pUser->getAudioKeeper()->getAudioMic();
    if (mic != nullptr) {
        mic->startSamplingForce();
    }
}

/**
 * Stops microphone sampling.
 * @param pUser Audio user.
 */
void stopMicSamplingForce(const IUseAudioKeeper* pUser) {
    if (pUser->getAudioKeeper()->isForceInvalidSe()) {
        return;
    }
    AudioMic* mic = pUser->getAudioKeeper()->getAudioMic();
    if (mic != nullptr) {
        mic->stopSamplingForce();
    }
}

/**
 * Invalidates microphone input.
 * @param pUser Audio user.
 */
void invalidateMicInput(const IUseAudioKeeper* pUser) {
    if (pUser->getAudioKeeper()->isForceInvalidSe()) {
        return;
    }
    AudioMic* mic = pUser->getAudioKeeper()->getAudioMic();
    if (mic != nullptr) {
        mic->invalidateInput();
    }
}

/**
 * Validates microphone input.
 * @param pUser Audio user.
 */
void validateMicInput(const IUseAudioKeeper* pUser) {
    if (pUser->getAudioKeeper()->isForceInvalidSe()) {
        return;
    }
    AudioMic* mic = pUser->getAudioKeeper()->getAudioMic();
    if (mic != nullptr) {
        mic->validateInput();
    }
}
}  // namespace al

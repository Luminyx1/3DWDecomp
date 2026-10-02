#include "Library/Audio/AudioMic.hpp"

#include <audio/seadAudioSettingParameter.h>
#include <audio/seadMicMgrCafe.h>

#include "Library/Audio/System/AudioMicBreathChecker.hpp"

namespace al {

/**
 * Constructs the microphone platform layer, registering a microphone manager with the audio
 * settings and allocating the sample buffers and the breath checker.
 * @param rParam Audio setting parameter the microphone manager is appended to.
 * @param sampleNum Number of samples read per update.
 */
AudioMicPlatform::AudioMicPlatform(sead::AudioSettingParameter& rParam, s32 sampleNum)
    : mSampleNum(sampleNum) {
    mHalfSampleNum = sampleNum >> 1;
    mMicMgr = new sead::MicMgrCafe();
    rParam.appendSubset(mMicMgr);
    mSampleBuffer = new (0x1000) u8[(mSampleNum >> 1) * 2];
    mWorkBuffer = new u8[0x800];
    mBreathChecker = new AudioMicBreathChecker();
}

/**
 * Finalizes the microphone. Does nothing on this platform.
 */
void AudioMicPlatform::finalize() {}

/**
 * Updates the microphone input. Does nothing on this platform.
 */
void AudioMicPlatform::update() {}

/**
 * Enables microphone input.
 */
void AudioMicPlatform::validateInput() {
    mIsValidInput = true;
}

/**
 * Disables microphone input.
 */
void AudioMicPlatform::invalidateInput() {
    mIsValidInput = false;
}

/**
 * Starts breath sampling and enables microphone input.
 */
void AudioMicPlatform::startSampling() {
    mBreathChecker->start();
    mIsValidInput = true;
}

/**
 * Starts breath sampling without changing whether input is enabled.
 */
void AudioMicPlatform::startSamplingForce() {
    mBreathChecker->start();
}

/**
 * Stops breath sampling.
 */
void AudioMicPlatform::stopSampling() {
    mBreathChecker->stop();
}

/**
 * Stops breath sampling.
 */
void AudioMicPlatform::stopSamplingForce() {
    mBreathChecker->stop();
}

/**
 * Checks whether a breath is currently being detected.
 * @return True if input is enabled and the breath checker detects a breath.
 */
bool AudioMicPlatform::isBreathInput() const {
    if (!mIsValidInput) {
        return false;
    }

    if (mBreathChecker == nullptr) {
        return false;
    }

    return mBreathChecker->isBreath();
}

/**
 * Gets the input power while a breath is detected.
 * @return The input power, 0 if no breath is detected, or -1 if there is no breath checker.
 */
f32 AudioMicPlatform::getBreathPower() const {
    if (mBreathChecker == nullptr) {
        return -1.0f;
    }

    if (!mIsValidInput || !mBreathChecker->isBreath()) {
        return 0.0f;
    }

    return mInputPower;
}

/**
 * Gets the input power ratio while a breath is detected.
 * @return The input power ratio, 0 if no breath is detected, or -1 if there is no breath checker.
 */
f32 AudioMicPlatform::getBreathPowerRatio() const {
    if (mBreathChecker == nullptr) {
        return -1.0f;
    }

    if (!mIsValidInput || !mBreathChecker->isBreath()) {
        return 0.0f;
    }

    return mInputPowerRatio;
}

/**
 * Checks whether there is loud enough microphone input or a breath.
 * @return True if the input is loud enough or a breath is detected.
 */
bool AudioMicPlatform::isMicInput() const {
    if (mInputPower == 0.0f) {
        return false;
    }

    if (mBreathChecker == nullptr || !mIsValidInput) {
        return false;
    }

    if (mInputPower > 850.0f) {
        return true;
    }

    return mBreathChecker->isBreath();
}

/**
 * Gets the microphone input power.
 * @return The input power, 0 if input is disabled, or -1 if there is no input.
 */
f32 AudioMicPlatform::getMicInputPower() const {
    if (mInputPower == 0.0f) {
        return -1.0f;
    }

    return mIsValidInput ? mInputPower : 0.0f;
}

/**
 * Gets the microphone input power ratio.
 * @return The input power ratio, 0 if input is disabled, or -1 if there is no input.
 */
f32 AudioMicPlatform::getMicInputPowerRatio() const {
    if (mInputPower == 0.0f) {
        return -1.0f;
    }

    return mIsValidInput ? mInputPowerRatio : 0.0f;
}

}  // namespace al

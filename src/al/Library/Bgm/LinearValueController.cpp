#include "Library/Bgm/LinearValueController.hpp"

#include <audio/seadSoundHandle.h>
#include <math/seadMathCalcCommon.h>

namespace al {
/**
 * Constructs a controller resting at a value.
 * @param value Initial value.
 */
LinearValueController::LinearValueController(f32 value) : mValue(value) {}

/**
 * Moves the value one step toward the target.
 */
void LinearValueController::update() {
    if (mStep > 0.0f && mTarget > mValue) {
        mValue += mStep;
        if (mTarget < mValue) {
            mValue = mTarget;
        }
    } else if (mStep < 0.0f && mTarget < mValue) {
        mValue += mStep;
        if (mTarget > mValue) {
            mValue = mTarget;
        }
    }
}

/**
 * Sets a new target.
 * @param target Target value.
 * @param speed Change per update.
 */
void LinearValueController::changeTarget(f32 target, f32 speed) {
    mTarget = target;
    mStep = mValue < target ? speed : -speed;
}

/**
 * Constructs a low pass filter controller.
 * @param pHandle Sound handle.
 */
BgmLpfController::BgmLpfController(sead::SoundHandle* pHandle) : mHandle(pHandle) {
    mFreqController = new LinearValueController(0.0f);
}

/**
 * Updates the cut off frequency of the sound.
 */
void BgmLpfController::update() {
    mFreqController->update();
    if (mIsReached) {
        return;
    }

    mHandle->SetLpfFreq(mFreqController->getValue());
    if (mFreqController->isReachedTarget()) {
        mIsReached = true;
    }
}

/**
 * Changes the cut off frequency.
 * @param freq Target frequency.
 * @param speed Change per update.
 */
void BgmLpfController::changeCutOffFreq(f32 freq, f32 speed) {
    mFreqController->changeTarget(freq, speed);
    mHandle->SetLpfFreq(mFreqController->getValue());
    mIsReached = mFreqController->isReachedTarget();
}

/**
 * Constructs a pitch controller.
 * @param pHandle Sound handle.
 */
BgmPitchController::BgmPitchController(sead::SoundHandle* pHandle) : mHandle(pHandle) {
    mPitchController = new LinearValueController(1.0f);
    mModulationDepthController = new LinearValueController(0.0f);
}

inline void BgmPitchController::applyPitch() {
    f32 modulation = sinf(mModulationPhase) * mModulationDepthController->getValue();
    f32 pitch = mPitchController->getValue();
    mHandle->SetPitch(pitch * exp2f(modulation));
    mModulationPhase += mModulationSpeed * (sead::Mathf::pi2() / 60.0f);
    while (mModulationPhase >= sead::Mathf::pi2()) {
        mModulationPhase -= sead::Mathf::pi2();
    }
}

/**
 * Updates the pitch of the sound.
 */
void BgmPitchController::update() {
    mPitchController->update();
    mModulationDepthController->update();
    applyPitch();
}

/**
 * Changes the pitch.
 * @param pitch Target pitch.
 * @param speed Change per update.
 */
void BgmPitchController::changePitch(f32 pitch, f32 speed) {
    mPitchController->changeTarget(pitch, speed);
    applyPitch();
}

/**
 * Changes the pitch modulation.
 * @param depth Target modulation depth.
 * @param depthSpeed Change of the depth per update.
 * @param speed Modulation speed.
 */
void BgmPitchController::changeModulation(f32 depth, f32 depthSpeed, f32 speed) {
    mModulationDepthController->changeTarget(depth, depthSpeed);
    mModulationSpeed = speed;
    mModulationPhase = 0.0f;
}

/**
 * Constructs a volume controller.
 * @param pHandle Sound handle.
 */
BgmVolumeController::BgmVolumeController(sead::SoundHandle* pHandle) : mHandle(pHandle) {
    mVolumeController = new LinearValueController(1.0f);
}

/**
 * Updates the volume of the sound.
 */
void BgmVolumeController::update() {
    mVolumeController->update();
    if (mIsReached) {
        return;
    }

    mHandle->SetVolume(mVolumeController->getValue(), 0);
    if (mVolumeController->isReachedTarget()) {
        mIsReached = true;
    }
}

/**
 * Changes the volume.
 * @param volume Target volume.
 * @param speed Change per update.
 */
void BgmVolumeController::changeVolume(f32 volume, f32 speed) {
    mVolumeController->changeTarget(volume, speed);
    mHandle->SetVolume(mVolumeController->getValue(), 0);
    mIsReached = mVolumeController->isReachedTarget();
}

/**
 * Checks whether the volume is fading out.
 * @return True if the volume is fading out.
 */
bool BgmVolumeController::isFadeOutNow() const {
    f32 value = mVolumeController->getValue();
    if (value <= 0.0f) {
        return false;
    }

    return value > mVolumeController->getTarget();
}

/**
 * Checks whether the volume is fading in.
 * @return True if the volume is fading in.
 */
bool BgmVolumeController::isFadeInNow() const {
    f32 value = mVolumeController->getValue();
    if (value >= 1.0f) {
        return false;
    }

    return value < mVolumeController->getTarget();
}

/**
 * Checks whether the volume has faded out.
 * @return True if the volume is zero.
 */
bool BgmVolumeController::isFinishedFadeOut() const {
    return mVolumeController->getValue() <= 0.0f;
}

/**
 * Gets the current volume.
 * @return Current volume.
 */
f32 BgmVolumeController::getCurFadeVolume() const {
    return mVolumeController->getValue();
}
}  // namespace al

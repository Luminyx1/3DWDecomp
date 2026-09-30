#include "Library/Se/Project/SeCategory.hpp"

namespace al {
/**
 * Constructs a mix volume at 0 dB.
 */
AudioMixVolume::AudioMixVolume() = default;

/**
 * Starts moving the volume.
 * @param volumeDb Target volume in decibels.
 * @param frames Length of the change in frames.
 */
void AudioMixVolume::moveTo(f32 volumeDb, s32 frames) {
    mTargetRatio = calcDecibelToRatio(volumeDb);
    mCurRatio = calcDecibelToRatio(mVolumeDb);
    mRemainFrames = frames;
    if (frames > 0) {
        mStep = (mTargetRatio - mCurRatio) / mRemainFrames;
    } else {
        mStep = 0.0f;
    }
}

/**
 * Advances the volume change.
 */
void AudioMixVolume::update() {
    if (mRemainFrames <= -1.0f) {
        return;
    }
    mRemainFrames += -1.0f;
    if (mRemainFrames <= 0.0f) {
        mVolumeDb = calcRatioToDecibel(mTargetRatio);
        mStep = 0.0f;
        mRemainFrames = -1.0f;
        return;
    }
    mCurRatio += mStep;
    mVolumeDb = calcRatioToDecibel(mCurRatio);
}

/**
 * Links this volume to another one that is added to it.
 * @param pVolume Linked volume.
 */
void AudioMixVolume::linkTo(const AudioMixVolume* pVolume) {
    mLinkedVolume = pVolume;
}

/**
 * Calculates the volume including all linked volumes.
 * @return Volume in decibels.
 */
f32 AudioMixVolume::calcLinkedVolumeDecibel() const {
    if (mLinkedVolume == nullptr) {
        return mVolumeDb;
    }
    return mVolumeDb + mLinkedVolume->calcLinkedVolumeDecibel();
}
}  // namespace al

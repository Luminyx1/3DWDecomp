#include "Library/Bgm/DspValueController.hpp"

#include <cmath>

namespace al {
/**
 * Constructs a controller at the given value.
 * @param value Initial value.
 */
DspLinearValueController::DspLinearValueController(f32 value) : mValue(value), mTarget(value), mStep(0.0f) {}

/**
 * Sets the value and the target to the given value.
 * @param value Value.
 */
void DspLinearValueController::init(f32 value) {
    mValue = value;
    mTarget = value;
}

/**
 * Moves the value towards the target.
 */
void DspLinearValueController::update() {
    if (mStep > 0.0f) {
        if (mTarget > mValue) {
            mValue += mStep;
            if (mTarget < mValue) {
                mValue = mTarget;
            }
        }
    }

    if (mStep < 0.0f) {
        if (mTarget < mValue) {
            mValue += mStep;
            if (mTarget > mValue) {
                mValue = mTarget;
            }
        }
    }
}

/**
 * Starts moving the value to a target.
 * @param target Target value.
 * @param frames Frames to reach the target, or 0 or less to set it immediately.
 */
void DspLinearValueController::changeTarget(f32 target, s32 frames) {
    f32 diff = target - mValue;
    mTarget = target;
    if (frames <= 0) {
        mValue = target;
        mStep = diff;
    } else {
        mStep = diff / frames;
    }
}

/**
 * Constructs a sine controller.
 * @param frameRate Frame rate.
 * @param value Initial value.
 */
DspSinValueController::DspSinValueController(f32 frameRate, f32 value)
    : mFrameRate(frameRate), mPhase(0.0f) {
    mFreq = 0.0f;
    mPhaseStep = 0.0f;
    mValue = value;
    mAmplitude = new DspLinearValueController(0.0f);
}

/**
 * Sets the value and resets the amplitude.
 * @param value Value.
 */
void DspSinValueController::init(f32 value) {
    mValue = value;
    mAmplitude->init(0.0f);
}

/**
 * Advances the sine wave.
 */
void DspSinValueController::update() {
    if (mFreq <= 0.0f) {
        return;
    }

    mAmplitude->update();
    mValue = sinf(mPhase) * mAmplitude->getValue();
    mPhase += mPhaseStep;
    while (mPhase >= 6.2831855f) {
        mPhase -= 6.2831855f;
    }
}

/**
 * Starts moving the amplitude to a target.
 * @param target Target amplitude.
 * @param frames Frames to reach the target.
 */
void DspSinValueController::changeTarget(f32 target, s32 frames) {
    mAmplitude->changeTarget(target, frames);
}

/**
 * Changes the frequency and restarts the wave.
 * @param freq Frequency.
 */
void DspSinValueController::changeFreq(f32 freq) {
    mPhase = 0.0f;
    mValue = 0.0f;
    mFreq = freq;
    mPhaseStep = 6.2831855f / mFrameRate * freq;
}
}  // namespace al

#include "Library/Audio/System/AudioMicBreathChecker.hpp"

#include <math/seadMathCalcCommon.h>

namespace al {

/**
 * Constructs an inactive breath checker with an empty detection history.
 */
AudioMicBreathChecker::AudioMicBreathChecker() = default;

/**
 * Analyzes a buffer of microphone samples if the checker is active.
 * @param pSamples Sample buffer.
 * @param sampleNum Number of samples in the buffer.
 */
void AudioMicBreathChecker::calc(s16* pSamples, s32 sampleNum) {
    if (mIsActive) {
        analyzeByBandPass(pSamples, sampleNum);
    }
}

/**
 * Detects breath by looking at how much the average of each 100-sample block fluctuates and at
 * the overall amplitude of the samples.
 * @param pSamples Sample buffer.
 * @param sampleNum Number of samples in the buffer.
 */
void AudioMicBreathChecker::analyzeByBandPass(s16* pSamples, s32 sampleNum) {
    s32 blockNum = sampleNum / 100;
    s32 blockIndex = 0;
    s32 prevAverage = 0;
    s32 absSum = 0;
    s32 diffSum = 0;

    for (s32 offset = 0; blockIndex < blockNum; blockIndex++, offset += 100) {
        s32 sum = 0;

        for (s32 j = 0; j < 100; j++) {
            s32 sample = pSamples[offset + j];
            sum += sample;
            absSum += sead::Mathi::abs(sample);
        }

        s32 average = sum / 100;
        diffSum += sead::Mathi::abs(average - prevAverage);
        prevAverage = average;
    }

    bool isDiffLarge = diffSum / blockNum > 1500;
    bool isAmplitudeLarge = absSum / (blockNum * 100) > 2500;
    bool isBreath = isDiffLarge & isAmplitudeLarge;
    pushHistory(isBreath);
    updateBreath();
}

/**
 * Activates the checker and clears its detection history.
 */
void AudioMicBreathChecker::start() {
    if (mIsActive) {
        return;
    }

    mIsActive = true;
    reset();
}

/**
 * Deactivates the checker.
 */
void AudioMicBreathChecker::stop() {
    if (mIsActive) {
        mIsActive = false;
        mIsBreath = false;
    }
}

/**
 * Detects breath by looking at how much the average of each 100-sample block deviates from the
 * average of the whole buffer and at the overall amplitude of the samples.
 * @param pSamples Sample buffer.
 * @param sampleNum Number of samples in the buffer.
 */
void AudioMicBreathChecker::analyzeByLowFreq(s16* pSamples, s32 sampleNum) {
    s32 blockNum = sampleNum / 100;
    s32 blockIndex = 0;
    s32 totalSum = 0;

    for (s32 offset = 0; blockIndex < blockNum; blockIndex++, offset += 100) {
        for (s32 j = 0; j < 100; j++) {
            totalSum += pSamples[offset + j];
        }
    }

    s32 totalAverage = totalSum / (blockNum * 100);
    s32 diffSum = 0;
    s32 absSum = 0;

    for (s32 i = 0, offset = 0; i < blockNum; i++, offset += 100) {
        s32 sum = 0;

        for (s32 j = 0; j < 100; j++) {
            s32 sample = pSamples[offset + j];
            sum += sample;
            absSum += sead::Mathi::abs(sample);
        }

        diffSum += sead::Mathi::abs(sum / 100 - totalAverage);
    }

    bool isDiffLarge = diffSum / blockNum > 800;
    bool isAmplitudeLarge = absSum / (blockNum * 100) > 2500;
    bool isBreath = isDiffLarge & isAmplitudeLarge;
    pushHistory(isBreath);
    updateBreath();
}

/**
 * Detects breath by counting zero crossings and loud samples in each 480-sample frame.
 * @param pSamples Sample buffer.
 * @param sampleNum Number of samples in the buffer.
 */
void AudioMicBreathChecker::analyzeByZeroCross(s16* pSamples, s32 sampleNum) {
    s32 frameNum = sampleNum / 480;
    s32 remainNum = sampleNum % 480;

    if (remainNum > 0) {
        frameNum++;
    }

    s32 frameIndex = 0;
    s32 offset = 0;

    for (; frameIndex < frameNum; frameIndex++) {
        s32 frameLength = 480;

        if (frameIndex == frameNum - 1 && remainNum != 0) {
            frameLength = remainNum;

            if (frameLength < 97) {
                break;
            }
        }

        s32 veryLoudNum = 0;
        s32 prevSample = 0;
        s32 positiveSum = 0;
        s32 zeroCrossNum = 0;
        s32 loudNum = 0;

        for (s32 j = 0; j < frameLength; j++) {
            s16 sample = pSamples[offset + j];
            s16 positiveSample = sample < 0 ? 0 : sample;
            positiveSum += positiveSample;

            if (prevSample < 0 && sample >= 0) {
                zeroCrossNum++;
            }

            if (sample > 700) {
                loudNum++;

                if (sample > 10000) {
                    veryLoudNum++;
                }
            }

            prevSample = sample;
        }

        s32 average = positiveSum / frameLength;
        s32 prevLoudNum = mPrevLoudSampleNum;
        mPrevLoudSampleNum = loudNum;
        bool isBreathByLoudChange = false;

        if (zeroCrossNum == 6 && average > 200 && average < 5000) {
            s32 loudChange = sead::Mathi::abs(loudNum - prevLoudNum);
            isBreathByLoudChange = loudChange > 10 && loudChange < 100;
        }

        bool isBreath = (veryLoudNum > 10 && zeroCrossNum < 10) || isBreathByLoudChange;
        pushHistory(isBreath);
        offset += frameLength;
    }

    updateBreath();
}

}  // namespace al

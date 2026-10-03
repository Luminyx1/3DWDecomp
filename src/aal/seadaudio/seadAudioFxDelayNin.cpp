#include "audio/seadAudioFxDelayNin.h"

#include <cstring>

#include "audio/seadAudioSystemNin.h"
#include "math/seadMathCalcCommon.h"

namespace sead {
namespace {
s32 sDelaySamplesPerFrame = 0;
}

/**
 * Constructs a delay parameter set with the default values.
 */
AudioFxDelayParamNin::AudioFxDelayParamNin()
    : mDelayTime(160.0f), mFeedbackGain(0.4f), mOutGain(1.0f), mLpfAmount(1.0f), mChannelCount(6),
      mSampleRate(0), _20(false), _24(8), _28(4) {
    if (sDelaySamplesPerFrame == 0) {
        sDelaySamplesPerFrame = AudioSystemNin::GetSamplesPerFrame();
    }
}

/**
 * Constructs a delay effect.
 */
AudioFxDelayNin::AudioFxDelayNin() {
    initVars_();
    SetAudioFrameCount(5);
}

/**
 * Releases the work buffer and clears the delay settings.
 */
void AudioFxDelayNin::initVars_() {
    ReleaseWorkBuffer();
    mDelaySize = 0;
    mBufferPos = 0;
    mFeedbackGain[0] = 0.0f;
    mFeedbackGain[1] = 0.0f;
    mLpfInGain[0] = 0.0f;
    mLpfInGain[1] = 0.0f;
    mLpfHistoryGain[0] = 0.0f;
    mLpfHistoryGain[1] = 0.0f;
    mOutGain[0] = 0.0f;
    mOutGain[1] = 0.0f;
}

/**
 * Finalizes and destroys the delay effect.
 */
AudioFxDelayNin::~AudioFxDelayNin() {
    Finalize();
}

/**
 * Clears the delay lines and starts the effect.
 * @return False if the effect was already initialized.
 */
bool AudioFxDelayNin::Initialize() {
    if (mIsInitialized) {
        return false;
    }

    clearBuffer_();
    mIsInitialized = true;
    return true;
}

/**
 * Clears the delay lines and the low-pass filter history.
 */
void AudioFxDelayNin::clearBuffer_() {
    u32 pairCount = mChannelCountMax / 2;

    for (u32 i = 0; i < pairCount; i++) {
        memset(mDelayBuffer[i], 0, mDelaySize * sizeof(Vector2f));
        mLpfHistory[i].set(0.0f, 0.0f);
    }

    initBufferPos_();
}

/**
 * Applies the delay to one audio frame block of samples.
 * @param pSamples Interleaved-by-channel sample buffer.
 * @param rArg Sample layout of the buffer.
 */
void AudioFxDelayNin::UpdateSamples(s32* pSamples, const UpdateSamplesArg& rArg) {
    mChannelCount = Mathu::clampMax(static_cast<u32>(rArg.channelCount), mChannelCountMax);
    s32 frameCount = rArg.readSampleCount / (rArg.sampleCountPerAudioFrame * rArg.channelCount);

    for (s32 i = 0; i < frameCount; i++) {
        s32 sampleCount = rArg.sampleCountPerAudioFrame;
        s32* ch0 = &pSamples[sampleCount * i * rArg.channelCount];
        s32* ch1 = ch0 + sampleCount;
        s32* ch2 = ch1 + sampleCount;
        s32* ch3 = ch2 + sampleCount;
        s32* ch4 = ch3 + sampleCount;
        s32* ch5 = ch4 + sampleCount;

        switch (mChannelCount) {
        case 2:
            updateFx2ch_(ch0, ch1, sampleCount);
            break;
        case 4:
            updateFx4ch_(ch0, ch1, ch2, ch3, sampleCount);
            break;
        case 6:
            updateFx6ch_(ch0, ch1, ch2, ch3, ch4, ch5, sampleCount);
            break;
        }
    }
}

/**
 * Applies the delay to a stereo pair.
 * @param pCh0 Front left samples.
 * @param pCh1 Front right samples.
 * @param sampleCount Number of samples per channel.
 */
void AudioFxDelayNin::updateFx2ch_(s32* pCh0, s32* pCh1, u32 sampleCount) {
    const f32 feedbackGain0 = mFeedbackGain[0];
    const f32 feedbackGain1 = mFeedbackGain[1];
    const f32 outGain0 = mOutGain[0];
    const f32 outGain1 = mOutGain[1];
    const f32 lpfInGain0 = mLpfInGain[0];
    const f32 lpfInGain1 = mLpfInGain[1];
    const f32 lpfHistoryGain0 = mLpfHistoryGain[0];
    const f32 lpfHistoryGain1 = mLpfHistoryGain[1];

    for (u32 i = 0; i < sampleCount; i++) {
        f32 in0 = *pCh0;
        f32 in1 = *pCh1;
        f32 input0 = lpfInGain0 * in0;
        f32 input1 = lpfInGain1 * in1;
        f32 history0 = lpfHistoryGain0 * mLpfHistory[0].x;
        f32 history1 = lpfHistoryGain1 * mLpfHistory[0].y;
        f32 lpf0x = input0 + history0;
        f32 lpf0y = input1 + history1;
        mLpfHistory[0].set(lpf0x, lpf0y);

        Vector2f* delay0 = &mDelayBuffer[0][mBufferPos];
        Vector2f out0 = *delay0;
        f32 feedback0 = feedbackGain0 * out0.x;
        f32 feedback1 = feedbackGain1 * out0.y;
        delay0->set(lpf0x + feedback0, lpf0y + feedback1);
        f32 res0 = outGain0 * out0.x;
        f32 res1 = outGain1 * out0.y;

        *pCh0++ = res0;
        *pCh1++ = res1;

        mBufferPos = mBufferPos + 1 >= mDelaySize ? 0 : mBufferPos + 1;
    }
}

/**
 * Applies the delay to two stereo pairs.
 * @param pCh0 Front left samples.
 * @param pCh1 Front right samples.
 * @param pCh2 Rear left samples.
 * @param pCh3 Rear right samples.
 * @param sampleCount Number of samples per channel.
 */
void AudioFxDelayNin::updateFx4ch_(s32* pCh0, s32* pCh1, s32* pCh2, s32* pCh3, u32 sampleCount) {
    const f32 feedbackGain0 = mFeedbackGain[0];
    const f32 feedbackGain1 = mFeedbackGain[1];
    const f32 outGain0 = mOutGain[0];
    const f32 outGain1 = mOutGain[1];
    const f32 lpfInGain0 = mLpfInGain[0];
    const f32 lpfInGain1 = mLpfInGain[1];
    const f32 lpfHistoryGain0 = mLpfHistoryGain[0];
    const f32 lpfHistoryGain1 = mLpfHistoryGain[1];

    for (u32 i = 0; i < sampleCount; i++) {
        f32 in0 = *pCh0;
        f32 in1 = *pCh1;
        f32 in2 = *pCh2;
        f32 in3 = *pCh3;
        f32 input0 = lpfInGain0 * in0;
        f32 input1 = lpfInGain1 * in1;
        f32 history0 = lpfHistoryGain0 * mLpfHistory[0].x;
        f32 history1 = lpfHistoryGain1 * mLpfHistory[0].y;
        f32 lpf0x = input0 + history0;
        f32 lpf0y = input1 + history1;
        mLpfHistory[0].set(lpf0x, lpf0y);
        f32 input2 = lpfInGain0 * in2;
        f32 input3 = lpfInGain1 * in3;
        f32 history2 = lpfHistoryGain0 * mLpfHistory[1].x;
        f32 history3 = lpfHistoryGain1 * mLpfHistory[1].y;
        f32 lpf1x = input2 + history2;
        f32 lpf1y = input3 + history3;
        mLpfHistory[1].set(lpf1x, lpf1y);

        Vector2f* delay0 = &mDelayBuffer[0][mBufferPos];
        Vector2f out0 = *delay0;
        f32 feedback0 = feedbackGain0 * out0.x;
        f32 feedback1 = feedbackGain1 * out0.y;
        delay0->set(lpf0x + feedback0, lpf0y + feedback1);
        f32 res0 = outGain0 * out0.x;
        f32 res1 = outGain1 * out0.y;

        Vector2f* delay1 = &mDelayBuffer[1][mBufferPos];
        Vector2f out1 = *delay1;
        f32 feedback2 = feedbackGain0 * out1.x;
        f32 feedback3 = feedbackGain1 * out1.y;
        delay1->set(lpf1x + feedback2, lpf1y + feedback3);
        f32 res2 = outGain0 * out1.x;
        f32 res3 = outGain1 * out1.y;

        *pCh0++ = res0;
        *pCh1++ = res1;
        *pCh2++ = res2;
        *pCh3++ = res3;

        mBufferPos = mBufferPos + 1 >= mDelaySize ? 0 : mBufferPos + 1;
    }
}

/**
 * Applies the delay to three stereo pairs.
 * @param pCh0 Front left samples.
 * @param pCh1 Front right samples.
 * @param pCh2 Rear left samples.
 * @param pCh3 Rear right samples.
 * @param pCh4 Front center samples.
 * @param pCh5 Low-frequency samples.
 * @param sampleCount Number of samples per channel.
 */
void AudioFxDelayNin::updateFx6ch_(s32* pCh0, s32* pCh1, s32* pCh2, s32* pCh3, s32* pCh4, s32* pCh5,
                                   u32 sampleCount) {
    const f32 feedbackGain0 = mFeedbackGain[0];
    const f32 feedbackGain1 = mFeedbackGain[1];
    const f32 outGain0 = mOutGain[0];
    const f32 outGain1 = mOutGain[1];
    const f32 lpfInGain0 = mLpfInGain[0];
    const f32 lpfInGain1 = mLpfInGain[1];
    const f32 lpfHistoryGain0 = mLpfHistoryGain[0];
    const f32 lpfHistoryGain1 = mLpfHistoryGain[1];

    for (u32 i = 0; i < sampleCount; i++) {
        f32 in0 = *pCh0;
        f32 in1 = *pCh1;
        f32 in2 = *pCh2;
        f32 in3 = *pCh3;
        f32 in4 = *pCh4;
        f32 in5 = *pCh5;
        f32 input0 = lpfInGain0 * in0;
        f32 input1 = lpfInGain1 * in1;
        f32 history0 = lpfHistoryGain0 * mLpfHistory[0].x;
        f32 history1 = lpfHistoryGain1 * mLpfHistory[0].y;
        f32 lpf0x = input0 + history0;
        f32 lpf0y = input1 + history1;
        mLpfHistory[0].set(lpf0x, lpf0y);
        f32 input2 = lpfInGain0 * in2;
        f32 input3 = lpfInGain1 * in3;
        f32 history2 = lpfHistoryGain0 * mLpfHistory[1].x;
        f32 history3 = lpfHistoryGain1 * mLpfHistory[1].y;
        f32 lpf1x = input2 + history2;
        f32 lpf1y = input3 + history3;
        mLpfHistory[1].set(lpf1x, lpf1y);
        f32 input4 = lpfInGain0 * in4;
        f32 input5 = lpfInGain1 * in5;
        f32 history4 = lpfHistoryGain0 * mLpfHistory[2].x;
        f32 history5 = lpfHistoryGain1 * mLpfHistory[2].y;
        f32 lpf2x = input4 + history4;
        f32 lpf2y = input5 + history5;
        mLpfHistory[2].set(lpf2x, lpf2y);

        Vector2f* delay0 = &mDelayBuffer[0][mBufferPos];
        Vector2f out0 = *delay0;
        f32 feedback0 = feedbackGain0 * out0.x;
        f32 feedback1 = feedbackGain1 * out0.y;
        delay0->set(lpf0x + feedback0, lpf0y + feedback1);
        f32 res0 = outGain0 * out0.x;
        f32 res1 = outGain1 * out0.y;

        Vector2f* delay1 = &mDelayBuffer[1][mBufferPos];
        Vector2f out1 = *delay1;
        f32 feedback2 = feedbackGain0 * out1.x;
        f32 feedback3 = feedbackGain1 * out1.y;
        delay1->set(lpf1x + feedback2, lpf1y + feedback3);
        f32 res2 = outGain0 * out1.x;
        f32 res3 = outGain1 * out1.y;

        Vector2f* delay2 = &mDelayBuffer[2][mBufferPos];
        Vector2f out2 = *delay2;
        f32 feedback4 = feedbackGain0 * out2.x;
        f32 feedback5 = feedbackGain1 * out2.y;
        delay2->set(lpf2x + feedback4, lpf2y + feedback5);
        f32 res4 = outGain0 * out2.x;
        f32 res5 = outGain1 * out2.y;

        *pCh0++ = res0;
        *pCh1++ = res1;
        *pCh2++ = res2;
        *pCh3++ = res3;
        *pCh4++ = res4;
        *pCh5++ = res5;

        mBufferPos = mBufferPos + 1 >= mDelaySize ? 0 : mBufferPos + 1;
    }
}

/**
 * Stops the effect.
 */
void AudioFxDelayNin::Finalize() {
    if (mIsInitialized) {
        mChannelCount = 0;
        mIsInitialized = false;
    }
}

/**
 * Applies a parameter set; delay sizes only change while no work buffer is assigned.
 * @param rParam Delay parameters.
 * @return Always true.
 */
bool AudioFxDelayNin::SetParam(const AudioFxDelayParamNin& rParam) {
    mChannelCountMax = rParam.mChannelCount;

    if (!mIsBufferAssigned) {
        setupDelaySizes_(rParam);
    }

    setupGains_(rParam);
    _128 = rParam._24;
    _12c = rParam._28;
    return true;
}

/**
 * Computes the delay line length from the delay time and sample rate.
 * @param rParam Delay parameters.
 */
void AudioFxDelayNin::setupDelaySizes_(const AudioFxDelayParamNin& rParam) {
    mSampleRate = rParam.mSampleRate;
    f32 delayTime = rParam.mDelayTime / 1000.0f;
    mDelaySize = getSampleRate_() * delayTime;
}

/**
 * Computes the output, feedback and low-pass filter gains.
 * @param rParam Delay parameters.
 */
void AudioFxDelayNin::setupGains_(const AudioFxDelayParamNin& rParam) {
    mOutGain[0] = mOutGain[1] = rParam.mOutGain;
    mFeedbackGain[0] = mFeedbackGain[1] = rParam.mFeedbackGain;
    f32 lpf = Mathf::clampMin(rParam.mLpfAmount, 0.05f);
    mLpfInGain[0] = lpf;
    mLpfInGain[1] = lpf;
    mLpfHistoryGain[0] = 1.0f - lpf;
    mLpfHistoryGain[1] = 1.0f - lpf;
}

/**
 * Gets the work buffer size needed for the aux buffer and all delay lines.
 * @return Required work buffer size in bytes.
 */
size_t AudioFxDelayNin::GetRequiredMemSize() const {
    u32 pairCount = mChannelCountMax / 2;
    u32 delaySize = (mDelaySize * sizeof(Vector2f) + 0x1f) & ~0x1f;
    u32 size = AudioFxBaseNin::GetRequiredMemSize() + delaySize * pairCount;
    return size + 0x20;
}

/**
 * Assigns the work buffer and splits it into the delay lines.
 * @param pBuffer Work buffer.
 * @param size Size of the work buffer.
 * @return False if a buffer is already assigned or the buffer is too small.
 */
bool AudioFxDelayNin::AssignWorkBuffer(void* pBuffer, u32 size) {
    if (mIsBufferAssigned) {
        return false;
    }

    AudioFxBaseNin::AssignWorkBuffer(pBuffer, size);
    uintptr_t start = reinterpret_cast<uintptr_t>(mFxWorkBuffer);
    uintptr_t current = (start + 0x1f) & ~0x1f;
    u32 pairCount = mChannelCountMax / 2;

    for (u32 i = 0; i < pairCount; i++) {
        mDelayBuffer[i] = reinterpret_cast<Vector2f*>(current);
        current = (reinterpret_cast<uintptr_t>(mDelayBuffer[i] + mDelaySize) + 0x1f) & ~0x1f;
    }

    if (static_cast<s64>(current - start) > size) {
        return false;
    }

    mIsBufferAssigned = true;
    return true;
}

/**
 * Detaches the delay lines from the work buffer and releases it.
 */
void AudioFxDelayNin::ReleaseWorkBuffer() {
    mIsBufferAssigned = false;

    for (u32 i = 0; i < cPairCountMax; i++) {
        mDelayBuffer[i] = nullptr;
    }

    for (u32 i = 0; i < 6; i++) {
        _f8[i] = nullptr;
    }

    AudioFxBaseNin::ReleaseWorkBuffer();
}

/**
 * Rewinds the delay line position.
 */
void AudioFxDelayNin::initBufferPos_() {
    mBufferPos = 0;
    _130 = 0;
    _138 = 0;
}

/**
 * Gets the sample rate selected by the parameters.
 * @return Sample rate in Hz, or 0 for an unknown setting.
 */
f32 AudioFxDelayNin::getSampleRate_() const {
    switch (mSampleRate) {
    case 0:
        return 32000.0f;
    case 1:
        return 48000.0f;
    default:
        return 0.0f;
    }
}
}  // namespace sead

#include "audio/seadAudioFxReverbStdNin.h"

#include <cmath>
#include <cstring>

#include "audio/seadAudioSystemNin.h"
#include "math/seadMathCalcCommon.h"

namespace sead {
namespace {
s32 sReverbStdSamplesPerFrame = 0;

const u32 cReverbStdEarlyDelay32k[8] = {163, 317, 479, 641, 797, 967, 1123, 1283};
const u32 cReverbStdEarlyDelay48k[8] = {241, 479, 719, 967, 1193, 1451, 1693, 1931};

const f32 cReverbStdEarlyCoefLow = -0.33f;
const f32 cReverbStdEarlyCoefHigh = 0.33f;

const u32 cReverbStdFusedDelay32k[6][4] = {
    {1789, 1999, 433, 149}, {149, 293, 251, 103},   {947, 1361, 433, 137},
    {1279, 1531, 509, 149}, {1531, 1847, 563, 179}, {1823, 2357, 571, 137},
};

const u32 cReverbStdFusedDelay48k[6][4] = {
    {2683, 2999, 647, 223}, {223, 439, 379, 157},   {1423, 2039, 647, 211},
    {1913, 2297, 761, 223}, {2297, 2777, 839, 269}, {2731, 3539, 857, 211},
};
}  // namespace

/**
 * Constructs a parameter set with the default values.
 */
AudioFxReverbStdParamNin::AudioFxReverbStdParamNin()
    : mPreDelayTime(0.02f), mDecayTime(3.0f), mColoration(0.6f), mLpfAmount(0.4f), mOutGain(1.0f),
      mEarlyMode(cEarlyMode_5), mFusedMode(0), mEarlyGain(0.0f), mFusedGain(1.0f), mChannelCount(6),
      mSampleRate(0), _34(false), _38(8), _3c(4) {
    if (sReverbStdSamplesPerFrame == 0) {
        sReverbStdSamplesPerFrame = AudioSystemNin::GetSamplesPerFrame();
    }
}

/**
 * Constructs the effect.
 */
AudioFxReverbStdNin::AudioFxReverbStdNin() {
    initVars_();
}

/**
 * Releases the work buffer and clears the delay settings and gains.
 */
void AudioFxReverbStdNin::initVars_() {
    ReleaseWorkBuffer();
    mEarlyDelaySize = 0;
    mEarlyPos = 0;
    mEarlyCoef[0] = 0.0f;
    mEarlyCoef[1] = 0.0f;
    mEarlyGain[0] = 0.0f;
    mEarlyGain[1] = 0.0f;
    mPreDelaySize = 0;
    mPreDelayPos = 0;
    for (u32 i = 0; i < cCombCount; i++) {
        mCombDelaySize[i] = 0;
        mCombPos[i] = 0;
        mCombCoef[i][0] = 0.0f;
        mCombCoef[i][1] = 0.0f;
    }
    for (u32 i = 0; i < cAllPassCount; i++) {
        mAllPassDelaySize[i] = 0;
        mAllPassPos[i] = 0;
    }
    mAllPassCoef[0] = 0.0f;
    mAllPassCoef[1] = 0.0f;
    mLpfInGain[0] = 0.0f;
    mLpfInGain[1] = 0.0f;
    mLpfHistoryGain[0] = 0.0f;
    mLpfHistoryGain[1] = 0.0f;
    mOutGain[0] = 0.0f;
    mOutGain[1] = 0.0f;
}

/**
 * Finalizes and destroys the effect.
 */
AudioFxReverbStdNin::~AudioFxReverbStdNin() {
    Finalize();
}

/**
 * Clears the delay lines and starts the effect.
 * @return False if the effect was already initialized.
 */
bool AudioFxReverbStdNin::Initialize() {
    if (mIsInitialized) {
        return false;
    }
    clearBuffer_();
    mIsInitialized = true;
    return true;
}

/**
 * Clears the delay lines and filter histories.
 */
void AudioFxReverbStdNin::clearBuffer_() {
    u32 pairCount = mChannelCountMax / 2;
    for (u32 i = 0; i < pairCount; i++) {
        memset(mEarlyBuffer[i], 0, mEarlyDelaySize * sizeof(Vector2f));
    }
    for (u32 i = 0; i < pairCount; i++) {
        if (mPreDelayBuffer[i]) {
            memset(mPreDelayBuffer[i], 0, mPreDelaySize * sizeof(Vector2f));
        }
    }
    for (u32 j = 0; j < cCombCount; j++) {
        for (u32 i = 0; i < pairCount; i++) {
            memset(mCombBuffer[j][i], 0, mCombDelaySize[j] * sizeof(Vector2f));
        }
    }
    for (u32 j = 0; j < cAllPassCount; j++) {
        for (u32 i = 0; i < pairCount; i++) {
            memset(mAllPassBuffer[j][i], 0, mAllPassDelaySize[j] * sizeof(Vector2f));
        }
    }
    for (u32 i = 0; i < pairCount; i++) {
        mLpfHistory[i].set(0.0f, 0.0f);
    }
    initBufferPos_();
}

/**
 * Applies the effect to one audio frame block of samples.
 * @param pSamples Sample buffer, one block per channel.
 * @param rArg Sample layout of the buffer.
 */
void AudioFxReverbStdNin::UpdateSamples(s32* pSamples, const UpdateSamplesArg& rArg) {
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
 * Applies the reverb to a stereo pair.
 * @param pCh0 Front left samples.
 * @param pCh1 Front right samples.
 * @param sampleCount Number of samples per channel.
 */
void AudioFxReverbStdNin::updateFx2ch_(s32* pCh0, s32* pCh1, u32 sampleCount) {
    const f32 earlyCoef0 = mEarlyCoef[0];
    const f32 earlyCoef1 = mEarlyCoef[1];
    const f32 earlyGain0 = mEarlyGain[0];
    const f32 earlyGain1 = mEarlyGain[1];
    f32 combCoef[cCombCount][2];
    memcpy(combCoef, mCombCoef, sizeof(combCoef));
    const f32 allPassCoef0 = mAllPassCoef[0];
    const f32 allPassCoef1 = mAllPassCoef[1];
    const f32 lpfInGain0 = mLpfInGain[0];
    const f32 lpfInGain1 = mLpfInGain[1];
    const f32 lpfHistoryGain0 = mLpfHistoryGain[0];
    const f32 lpfHistoryGain1 = mLpfHistoryGain[1];
    const f32 outGain0 = mOutGain[0];
    const f32 outGain1 = mOutGain[1];
    for (u32 i = 0; i < sampleCount; i++) {
        f32 in0x = *pCh0;
        f32 in0y = *pCh1;
        Vector2f* early0 = &mEarlyBuffer[0][mEarlyPos];
        f32 early0x = early0->x;
        f32 early0y = early0->y;
        early0->set(earlyCoef0 * early0x + in0x, earlyCoef1 * early0y + in0y);
        f32 earlyOut0x = earlyGain0 * early0x;
        f32 earlyOut0y = earlyGain1 * early0y;
        if (mPreDelaySize != 0) {
            Vector2f* preDelay0 = &mPreDelayBuffer[0][mPreDelayPos];
            f32 delayed0x = preDelay0->x;
            f32 delayed0y = preDelay0->y;
            preDelay0->set(in0x, in0y);
            in0x = delayed0x;
            in0y = delayed0y;
        }
        Vector2f* comb00 = &mCombBuffer[0][0][mCombPos[0]];
        f32 comb00x = comb00->x;
        f32 comb00y = comb00->y;
        comb00->set(in0x + comb00x * combCoef[0][0], in0y + comb00y * combCoef[0][1]);
        Vector2f* comb10 = &mCombBuffer[1][0][mCombPos[1]];
        f32 comb10x = comb10->x;
        f32 comb10y = comb10->y;
        comb10->set(in0x + comb10x * combCoef[1][0], in0y + comb10y * combCoef[1][1]);
        f32 out0x = comb00x + comb10x;
        f32 out0y = comb00y + comb10y;
        Vector2f* allPass00 = &mAllPassBuffer[0][0][mAllPassPos[0]];
        f32 allPass00x = allPass00->x;
        f32 allPass00y = allPass00->y;
        f32 temp00x = out0x + allPassCoef0 * allPass00x;
        f32 temp00y = out0y + allPassCoef1 * allPass00y;
        allPass00->set(temp00x, temp00y);
        out0x = allPassCoef0 * temp00x - allPass00x;
        out0y = allPassCoef1 * temp00y - allPass00y;
        f32 lpf0x = lpfHistoryGain0 * mLpfHistory[0].x + lpfInGain0 * out0x;
        f32 lpf0y = lpfHistoryGain1 * mLpfHistory[0].y + lpfInGain1 * out0y;
        mLpfHistory[0].x = lpf0x;
        mLpfHistory[0].y = lpf0y;
        Vector2f* allPass10 = &mAllPassBuffer[1][0][mAllPassPos[1]];
        f32 allPass10x = allPass10->x;
        f32 allPass10y = allPass10->y;
        f32 temp10x = lpf0x + allPassCoef0 * allPass10x;
        f32 temp10y = lpf0y + allPassCoef1 * allPass10y;
        allPass10->set(temp10x, temp10y);
        out0x = allPassCoef0 * temp10x - allPass10x;
        out0y = allPassCoef1 * temp10y - allPass10y;
        *pCh0++ = earlyOut0x + outGain0 * out0x;
        *pCh1++ = earlyOut0y + outGain1 * out0y;

        mEarlyPos = mEarlyPos + 1 >= mEarlyDelaySize ? 0 : mEarlyPos + 1;
        if (mPreDelaySize != 0) {
            mPreDelayPos = mPreDelayPos + 1 >= mPreDelaySize ? 0 : mPreDelayPos + 1;
        }
        for (u32 j = 0; j < cCombCount; j++) {
            mCombPos[j] = mCombPos[j] + 1 >= mCombDelaySize[j] ? 0 : mCombPos[j] + 1;
        }
        for (u32 j = 0; j < cAllPassCount; j++) {
            mAllPassPos[j] = mAllPassPos[j] + 1 >= mAllPassDelaySize[j] ? 0 : mAllPassPos[j] + 1;
        }
    }
}

/**
 * Applies the reverb to two stereo pairs.
 * @param pCh0 Front left samples.
 * @param pCh1 Front right samples.
 * @param pCh2 Rear left samples.
 * @param pCh3 Rear right samples.
 * @param sampleCount Number of samples per channel.
 */
void AudioFxReverbStdNin::updateFx4ch_(s32* pCh0, s32* pCh1, s32* pCh2, s32* pCh3, u32 sampleCount) {
    const f32 earlyCoef0 = mEarlyCoef[0];
    const f32 earlyCoef1 = mEarlyCoef[1];
    const f32 earlyGain0 = mEarlyGain[0];
    const f32 earlyGain1 = mEarlyGain[1];
    f32 combCoef[cCombCount][2];
    memcpy(combCoef, mCombCoef, sizeof(combCoef));
    const f32 allPassCoef0 = mAllPassCoef[0];
    const f32 allPassCoef1 = mAllPassCoef[1];
    const f32 lpfInGain0 = mLpfInGain[0];
    const f32 lpfInGain1 = mLpfInGain[1];
    const f32 lpfHistoryGain0 = mLpfHistoryGain[0];
    const f32 lpfHistoryGain1 = mLpfHistoryGain[1];
    const f32 outGain0 = mOutGain[0];
    const f32 outGain1 = mOutGain[1];
    for (u32 i = 0; i < sampleCount; i++) {
        f32 in0x = *pCh0;
        f32 in0y = *pCh1;
        f32 in1x = *pCh2;
        f32 in1y = *pCh3;
        Vector2f* early0 = &mEarlyBuffer[0][mEarlyPos];
        f32 early0x = early0->x;
        f32 early0y = early0->y;
        early0->set(earlyCoef0 * early0x + in0x, earlyCoef1 * early0y + in0y);
        f32 earlyOut0x = earlyGain0 * early0x;
        f32 earlyOut0y = earlyGain1 * early0y;
        Vector2f* early1 = &mEarlyBuffer[1][mEarlyPos];
        f32 early1x = early1->x;
        f32 early1y = early1->y;
        early1->set(earlyCoef0 * early1x + in1x, earlyCoef1 * early1y + in1y);
        f32 earlyOut1x = earlyGain0 * early1x;
        f32 earlyOut1y = earlyGain1 * early1y;
        if (mPreDelaySize != 0) {
            Vector2f* preDelay0 = &mPreDelayBuffer[0][mPreDelayPos];
            f32 delayed0x = preDelay0->x;
            f32 delayed0y = preDelay0->y;
            preDelay0->set(in0x, in0y);
            in0x = delayed0x;
            in0y = delayed0y;
            Vector2f* preDelay1 = &mPreDelayBuffer[1][mPreDelayPos];
            f32 delayed1x = preDelay1->x;
            f32 delayed1y = preDelay1->y;
            preDelay1->set(in1x, in1y);
            in1x = delayed1x;
            in1y = delayed1y;
        }
        Vector2f* comb00 = &mCombBuffer[0][0][mCombPos[0]];
        f32 comb00x = comb00->x;
        f32 comb00y = comb00->y;
        comb00->set(in0x + comb00x * combCoef[0][0], in0y + comb00y * combCoef[0][1]);
        Vector2f* comb01 = &mCombBuffer[0][1][mCombPos[0]];
        f32 comb01x = comb01->x;
        f32 comb01y = comb01->y;
        comb01->set(in1x + comb01x * combCoef[0][0], in1y + comb01y * combCoef[0][1]);
        Vector2f* comb10 = &mCombBuffer[1][0][mCombPos[1]];
        f32 comb10x = comb10->x;
        f32 comb10y = comb10->y;
        comb10->set(in0x + comb10x * combCoef[1][0], in0y + comb10y * combCoef[1][1]);
        Vector2f* comb11 = &mCombBuffer[1][1][mCombPos[1]];
        f32 comb11x = comb11->x;
        f32 comb11y = comb11->y;
        comb11->set(in1x + comb11x * combCoef[1][0], in1y + comb11y * combCoef[1][1]);
        f32 out0x = comb00x + comb10x;
        f32 out0y = comb00y + comb10y;
        f32 out1x = comb01x + comb11x;
        f32 out1y = comb01y + comb11y;
        Vector2f* allPass00 = &mAllPassBuffer[0][0][mAllPassPos[0]];
        f32 allPass00x = allPass00->x;
        f32 allPass00y = allPass00->y;
        f32 temp00x = out0x + allPassCoef0 * allPass00x;
        f32 temp00y = out0y + allPassCoef1 * allPass00y;
        allPass00->set(temp00x, temp00y);
        out0x = allPassCoef0 * temp00x - allPass00x;
        out0y = allPassCoef1 * temp00y - allPass00y;
        Vector2f* allPass01 = &mAllPassBuffer[0][1][mAllPassPos[0]];
        f32 allPass01x = allPass01->x;
        f32 allPass01y = allPass01->y;
        f32 temp01x = out1x + allPassCoef0 * allPass01x;
        f32 temp01y = out1y + allPassCoef1 * allPass01y;
        allPass01->set(temp01x, temp01y);
        out1x = allPassCoef0 * temp01x - allPass01x;
        out1y = allPassCoef1 * temp01y - allPass01y;
        f32 lpf0x = lpfHistoryGain0 * mLpfHistory[0].x + lpfInGain0 * out0x;
        f32 lpf0y = lpfHistoryGain1 * mLpfHistory[0].y + lpfInGain1 * out0y;
        mLpfHistory[0].x = lpf0x;
        mLpfHistory[0].y = lpf0y;
        f32 lpf1x = lpfHistoryGain0 * mLpfHistory[1].x + lpfInGain0 * out1x;
        f32 lpf1y = lpfHistoryGain1 * mLpfHistory[1].y + lpfInGain1 * out1y;
        mLpfHistory[1].x = lpf1x;
        mLpfHistory[1].y = lpf1y;
        Vector2f* allPass10 = &mAllPassBuffer[1][0][mAllPassPos[1]];
        f32 allPass10x = allPass10->x;
        f32 allPass10y = allPass10->y;
        f32 temp10x = lpf0x + allPassCoef0 * allPass10x;
        f32 temp10y = lpf0y + allPassCoef1 * allPass10y;
        allPass10->set(temp10x, temp10y);
        out0x = allPassCoef0 * temp10x - allPass10x;
        out0y = allPassCoef1 * temp10y - allPass10y;
        Vector2f* allPass11 = &mAllPassBuffer[1][1][mAllPassPos[1]];
        f32 allPass11x = allPass11->x;
        f32 allPass11y = allPass11->y;
        f32 temp11x = lpf1x + allPassCoef0 * allPass11x;
        f32 temp11y = lpf1y + allPassCoef1 * allPass11y;
        allPass11->set(temp11x, temp11y);
        out1x = allPassCoef0 * temp11x - allPass11x;
        out1y = allPassCoef1 * temp11y - allPass11y;
        *pCh0++ = earlyOut0x + outGain0 * out0x;
        *pCh1++ = earlyOut0y + outGain1 * out0y;
        *pCh2++ = earlyOut1x + outGain0 * out1x;
        *pCh3++ = earlyOut1y + outGain1 * out1y;

        mEarlyPos = mEarlyPos + 1 >= mEarlyDelaySize ? 0 : mEarlyPos + 1;
        if (mPreDelaySize != 0) {
            mPreDelayPos = mPreDelayPos + 1 >= mPreDelaySize ? 0 : mPreDelayPos + 1;
        }
        for (u32 j = 0; j < cCombCount; j++) {
            mCombPos[j] = mCombPos[j] + 1 >= mCombDelaySize[j] ? 0 : mCombPos[j] + 1;
        }
        for (u32 j = 0; j < cAllPassCount; j++) {
            mAllPassPos[j] = mAllPassPos[j] + 1 >= mAllPassDelaySize[j] ? 0 : mAllPassPos[j] + 1;
        }
    }
}

/**
 * Applies the reverb to three stereo pairs.
 * @param pCh0 Front left samples.
 * @param pCh1 Front right samples.
 * @param pCh2 Rear left samples.
 * @param pCh3 Rear right samples.
 * @param pCh4 Front center samples.
 * @param pCh5 Low-frequency samples.
 * @param sampleCount Number of samples per channel.
 */
void AudioFxReverbStdNin::updateFx6ch_(s32* pCh0, s32* pCh1, s32* pCh2, s32* pCh3, s32* pCh4, s32* pCh5, u32 sampleCount) {
    const f32 earlyCoef0 = mEarlyCoef[0];
    const f32 earlyCoef1 = mEarlyCoef[1];
    const f32 earlyGain0 = mEarlyGain[0];
    const f32 earlyGain1 = mEarlyGain[1];
    f32 combCoef[cCombCount][2];
    memcpy(combCoef, mCombCoef, sizeof(combCoef));
    const f32 allPassCoef0 = mAllPassCoef[0];
    const f32 allPassCoef1 = mAllPassCoef[1];
    const f32 lpfInGain0 = mLpfInGain[0];
    const f32 lpfInGain1 = mLpfInGain[1];
    const f32 lpfHistoryGain0 = mLpfHistoryGain[0];
    const f32 lpfHistoryGain1 = mLpfHistoryGain[1];
    const f32 outGain0 = mOutGain[0];
    const f32 outGain1 = mOutGain[1];
    for (u32 i = 0; i < sampleCount; i++) {
        f32 in0x = *pCh0;
        f32 in0y = *pCh1;
        f32 in1x = *pCh2;
        f32 in1y = *pCh3;
        f32 in2x = *pCh4;
        f32 in2y = *pCh5;
        Vector2f* early0 = &mEarlyBuffer[0][mEarlyPos];
        f32 early0x = early0->x;
        f32 early0y = early0->y;
        early0->set(earlyCoef0 * early0x + in0x, earlyCoef1 * early0y + in0y);
        f32 earlyOut0x = earlyGain0 * early0x;
        f32 earlyOut0y = earlyGain1 * early0y;
        Vector2f* early1 = &mEarlyBuffer[1][mEarlyPos];
        f32 early1x = early1->x;
        f32 early1y = early1->y;
        early1->set(earlyCoef0 * early1x + in1x, earlyCoef1 * early1y + in1y);
        f32 earlyOut1x = earlyGain0 * early1x;
        f32 earlyOut1y = earlyGain1 * early1y;
        Vector2f* early2 = &mEarlyBuffer[2][mEarlyPos];
        f32 early2x = early2->x;
        f32 early2y = early2->y;
        early2->set(earlyCoef0 * early2x + in2x, earlyCoef1 * early2y + in2y);
        f32 earlyOut2x = earlyGain0 * early2x;
        f32 earlyOut2y = earlyGain1 * early2y;
        if (mPreDelaySize != 0) {
            Vector2f* preDelay0 = &mPreDelayBuffer[0][mPreDelayPos];
            f32 delayed0x = preDelay0->x;
            f32 delayed0y = preDelay0->y;
            preDelay0->set(in0x, in0y);
            in0x = delayed0x;
            in0y = delayed0y;
            Vector2f* preDelay1 = &mPreDelayBuffer[1][mPreDelayPos];
            f32 delayed1x = preDelay1->x;
            f32 delayed1y = preDelay1->y;
            preDelay1->set(in1x, in1y);
            in1x = delayed1x;
            in1y = delayed1y;
            Vector2f* preDelay2 = &mPreDelayBuffer[2][mPreDelayPos];
            f32 delayed2x = preDelay2->x;
            f32 delayed2y = preDelay2->y;
            preDelay2->set(in2x, in2y);
            in2x = delayed2x;
            in2y = delayed2y;
        }
        Vector2f* comb00 = &mCombBuffer[0][0][mCombPos[0]];
        f32 comb00x = comb00->x;
        f32 comb00y = comb00->y;
        comb00->set(in0x + comb00x * combCoef[0][0], in0y + comb00y * combCoef[0][1]);
        Vector2f* comb01 = &mCombBuffer[0][1][mCombPos[0]];
        f32 comb01x = comb01->x;
        f32 comb01y = comb01->y;
        comb01->set(in1x + comb01x * combCoef[0][0], in1y + comb01y * combCoef[0][1]);
        Vector2f* comb02 = &mCombBuffer[0][2][mCombPos[0]];
        f32 comb02x = comb02->x;
        f32 comb02y = comb02->y;
        comb02->set(in2x + comb02x * combCoef[0][0], in2y + comb02y * combCoef[0][1]);
        Vector2f* comb10 = &mCombBuffer[1][0][mCombPos[1]];
        f32 comb10x = comb10->x;
        f32 comb10y = comb10->y;
        comb10->set(in0x + comb10x * combCoef[1][0], in0y + comb10y * combCoef[1][1]);
        Vector2f* comb11 = &mCombBuffer[1][1][mCombPos[1]];
        f32 comb11x = comb11->x;
        f32 comb11y = comb11->y;
        comb11->set(in1x + comb11x * combCoef[1][0], in1y + comb11y * combCoef[1][1]);
        Vector2f* comb12 = &mCombBuffer[1][2][mCombPos[1]];
        f32 comb12x = comb12->x;
        f32 comb12y = comb12->y;
        comb12->set(in2x + comb12x * combCoef[1][0], in2y + comb12y * combCoef[1][1]);
        f32 out0x = comb00x + comb10x;
        f32 out0y = comb00y + comb10y;
        f32 out1x = comb01x + comb11x;
        f32 out1y = comb01y + comb11y;
        f32 out2x = comb02x + comb12x;
        f32 out2y = comb02y + comb12y;
        Vector2f* allPass00 = &mAllPassBuffer[0][0][mAllPassPos[0]];
        f32 allPass00x = allPass00->x;
        f32 allPass00y = allPass00->y;
        f32 temp00x = out0x + allPassCoef0 * allPass00x;
        f32 temp00y = out0y + allPassCoef1 * allPass00y;
        allPass00->set(temp00x, temp00y);
        out0x = allPassCoef0 * temp00x - allPass00x;
        out0y = allPassCoef1 * temp00y - allPass00y;
        Vector2f* allPass01 = &mAllPassBuffer[0][1][mAllPassPos[0]];
        f32 allPass01x = allPass01->x;
        f32 allPass01y = allPass01->y;
        f32 temp01x = out1x + allPassCoef0 * allPass01x;
        f32 temp01y = out1y + allPassCoef1 * allPass01y;
        allPass01->set(temp01x, temp01y);
        out1x = allPassCoef0 * temp01x - allPass01x;
        out1y = allPassCoef1 * temp01y - allPass01y;
        Vector2f* allPass02 = &mAllPassBuffer[0][2][mAllPassPos[0]];
        f32 allPass02x = allPass02->x;
        f32 allPass02y = allPass02->y;
        f32 temp02x = out2x + allPassCoef0 * allPass02x;
        f32 temp02y = out2y + allPassCoef1 * allPass02y;
        allPass02->set(temp02x, temp02y);
        out2x = allPassCoef0 * temp02x - allPass02x;
        out2y = allPassCoef1 * temp02y - allPass02y;
        f32 lpf0x = lpfHistoryGain0 * mLpfHistory[0].x + lpfInGain0 * out0x;
        f32 lpf0y = lpfHistoryGain1 * mLpfHistory[0].y + lpfInGain1 * out0y;
        mLpfHistory[0].x = lpf0x;
        mLpfHistory[0].y = lpf0y;
        f32 lpf1x = lpfHistoryGain0 * mLpfHistory[1].x + lpfInGain0 * out1x;
        f32 lpf1y = lpfHistoryGain1 * mLpfHistory[1].y + lpfInGain1 * out1y;
        mLpfHistory[1].x = lpf1x;
        mLpfHistory[1].y = lpf1y;
        f32 lpf2x = lpfHistoryGain0 * mLpfHistory[2].x + lpfInGain0 * out2x;
        f32 lpf2y = lpfHistoryGain1 * mLpfHistory[2].y + lpfInGain1 * out2y;
        mLpfHistory[2].x = lpf2x;
        mLpfHistory[2].y = lpf2y;
        Vector2f* allPass10 = &mAllPassBuffer[1][0][mAllPassPos[1]];
        f32 allPass10x = allPass10->x;
        f32 allPass10y = allPass10->y;
        f32 temp10x = lpf0x + allPassCoef0 * allPass10x;
        f32 temp10y = lpf0y + allPassCoef1 * allPass10y;
        allPass10->set(temp10x, temp10y);
        out0x = allPassCoef0 * temp10x - allPass10x;
        out0y = allPassCoef1 * temp10y - allPass10y;
        Vector2f* allPass11 = &mAllPassBuffer[1][1][mAllPassPos[1]];
        f32 allPass11x = allPass11->x;
        f32 allPass11y = allPass11->y;
        f32 temp11x = lpf1x + allPassCoef0 * allPass11x;
        f32 temp11y = lpf1y + allPassCoef1 * allPass11y;
        allPass11->set(temp11x, temp11y);
        out1x = allPassCoef0 * temp11x - allPass11x;
        out1y = allPassCoef1 * temp11y - allPass11y;
        Vector2f* allPass12 = &mAllPassBuffer[1][2][mAllPassPos[1]];
        f32 allPass12x = allPass12->x;
        f32 allPass12y = allPass12->y;
        f32 temp12x = lpf2x + allPassCoef0 * allPass12x;
        f32 temp12y = lpf2y + allPassCoef1 * allPass12y;
        allPass12->set(temp12x, temp12y);
        out2x = allPassCoef0 * temp12x - allPass12x;
        out2y = allPassCoef1 * temp12y - allPass12y;
        *pCh0++ = earlyOut0x + outGain0 * out0x;
        *pCh1++ = earlyOut0y + outGain1 * out0y;
        *pCh2++ = earlyOut1x + outGain0 * out1x;
        *pCh3++ = earlyOut1y + outGain1 * out1y;
        *pCh4++ = earlyOut2x + outGain0 * out2x;
        *pCh5++ = earlyOut2y + outGain1 * out2y;

        mEarlyPos = mEarlyPos + 1 >= mEarlyDelaySize ? 0 : mEarlyPos + 1;
        if (mPreDelaySize != 0) {
            mPreDelayPos = mPreDelayPos + 1 >= mPreDelaySize ? 0 : mPreDelayPos + 1;
        }
        for (u32 j = 0; j < cCombCount; j++) {
            mCombPos[j] = mCombPos[j] + 1 >= mCombDelaySize[j] ? 0 : mCombPos[j] + 1;
        }
        for (u32 j = 0; j < cAllPassCount; j++) {
            mAllPassPos[j] = mAllPassPos[j] + 1 >= mAllPassDelaySize[j] ? 0 : mAllPassPos[j] + 1;
        }
    }
}

/**
 * Stops the effect.
 */
void AudioFxReverbStdNin::Finalize() {
    if (mIsInitialized) {
        mChannelCount = 0;
        mIsInitialized = false;
    }
}

/**
 * Applies a parameter set; delay sizes only change while no work buffer is assigned.
 * @param rParam Reverb parameters.
 * @return Always true.
 */
bool AudioFxReverbStdNin::SetParam(const AudioFxReverbStdParamNin& rParam) {
    mChannelCountMax = rParam.mChannelCount;
    if (!mIsBufferAssigned) {
        setupDelaySizes_(rParam);
    }
    setupGains_(rParam);
    _1e8 = rParam._38;
    _1ec = rParam._3c;
    return true;
}

/**
 * Computes the delay line lengths from the parameters and sample rate.
 * @param rParam Reverb parameters.
 */
void AudioFxReverbStdNin::setupDelaySizes_(const AudioFxReverbStdParamNin& rParam) {
    mSampleRate = rParam.mSampleRate;
    const u32* earlyTable = mSampleRate == 0 ? cReverbStdEarlyDelay32k : cReverbStdEarlyDelay48k;
    mEarlyDelaySize = earlyTable[rParam.mEarlyMode];
    const f32& earlyCoef = rParam.mEarlyMode < AudioFxReverbStdParamNin::cEarlyMode_4 ?
                               cReverbStdEarlyCoefLow :
                               cReverbStdEarlyCoefHigh;
    mEarlyCoef[0] = earlyCoef;
    mEarlyCoef[1] = earlyCoef;
    mPreDelaySize = getSampleRate_() * rParam.mPreDelayTime;
    const u32(*fusedTable)[4] = mSampleRate == 0 ? cReverbStdFusedDelay32k : cReverbStdFusedDelay48k;
    const u32* fused = fusedTable[rParam.mFusedMode];
    mCombDelaySize[0] = fused[0];
    mCombDelaySize[1] = fused[1];
    mAllPassDelaySize[0] = fused[2];
    mAllPassDelaySize[1] = fused[3];
}

/**
 * Computes the filter coefficients and gains from the parameters.
 * @param rParam Reverb parameters.
 */
void AudioFxReverbStdNin::setupGains_(const AudioFxReverbStdParamNin& rParam) {
    f32 outGain = rParam.mOutGain * 0.6f;
    mEarlyGain[0] = outGain * rParam.mEarlyGain;
    mEarlyGain[1] = mEarlyGain[0];
    for (u32 i = 0; i < cCombCount; i++) {
        mCombCoef[i][0] = std::pow(
            10.0f, (mCombDelaySize[i] * -3.0f) / (rParam.mDecayTime * getSampleRate_()));
        mCombCoef[i][1] = mCombCoef[i][0];
    }
    mAllPassCoef[0] = rParam.mColoration;
    mAllPassCoef[1] = mAllPassCoef[0];
    f32 lpf = Mathf::clampMin(rParam.mLpfAmount, 0.05f);
    mLpfInGain[0] = lpf;
    mLpfInGain[1] = lpf;
    mLpfHistoryGain[0] = 1.0f - lpf;
    mLpfHistoryGain[1] = 1.0f - lpf;
    mOutGain[0] = outGain * rParam.mFusedGain;
    mOutGain[1] = mOutGain[0];
}

/**
 * Gets the work buffer size needed for the aux buffer and all delay lines.
 * @return Required work buffer size in bytes.
 */
size_t AudioFxReverbStdNin::GetRequiredMemSize() const {
    u32 pairCount = mChannelCountMax / 2;
    u32 delaySize = ((mEarlyDelaySize * sizeof(Vector2f) + 0x1f) & ~0x1f) +
                    ((mPreDelaySize * sizeof(Vector2f) + 0x1f) & ~0x1f);
    u32 combSize = 0;
    for (u32 i = 0; i < cCombCount; i++) {
        combSize += (mCombDelaySize[i] * sizeof(Vector2f) + 0x1f) & ~0x1f;
        combSize *= pairCount;
    }
    u32 allPassSize = 0;
    for (u32 i = 0; i < cAllPassCount; i++) {
        allPassSize += (mAllPassDelaySize[i] * sizeof(Vector2f) + 0x1f) & ~0x1f;
        allPassSize *= pairCount;
    }
    u32 size = AudioFxBaseNin::GetRequiredMemSize();
    size += delaySize * pairCount + combSize + allPassSize + 0x20;
    return size;
}

/**
 * Assigns the work buffer and splits it into the delay lines.
 * @param pBuffer Work buffer.
 * @param size Size of the work buffer.
 * @return False if a buffer is already assigned or the buffer is too small.
 */
bool AudioFxReverbStdNin::AssignWorkBuffer(void* pBuffer, u32 size) {
    if (mIsBufferAssigned) {
        return false;
    }
    AudioFxBaseNin::AssignWorkBuffer(pBuffer, size);
    uintptr_t start = reinterpret_cast<uintptr_t>(mFxWorkBuffer);
    uintptr_t current = (start + 0x1f) & ~0x1f;
    u32 pairCount = mChannelCountMax / 2;
    for (u32 i = 0; i < pairCount; i++) {
        mEarlyBuffer[i] = reinterpret_cast<Vector2f*>(current);
        current = (reinterpret_cast<uintptr_t>(mEarlyBuffer[i] + mEarlyDelaySize) + 0x1f) & ~0x1f;
    }
    if (mPreDelaySize != 0) {
        for (u32 i = 0; i < pairCount; i++) {
            mPreDelayBuffer[i] = reinterpret_cast<Vector2f*>(current);
            current =
                (reinterpret_cast<uintptr_t>(mPreDelayBuffer[i] + mPreDelaySize) + 0x1f) & ~0x1f;
        }
    }
    for (u32 j = 0; j < cCombCount; j++) {
        for (u32 i = 0; i < pairCount; i++) {
            mCombBuffer[j][i] = reinterpret_cast<Vector2f*>(current);
            current = (reinterpret_cast<uintptr_t>(mCombBuffer[j][i] + mCombDelaySize[j]) + 0x1f) &
                      ~0x1f;
        }
    }
    for (u32 j = 0; j < cAllPassCount; j++) {
        for (u32 i = 0; i < pairCount; i++) {
            mAllPassBuffer[j][i] = reinterpret_cast<Vector2f*>(current);
            current =
                (reinterpret_cast<uintptr_t>(mAllPassBuffer[j][i] + mAllPassDelaySize[j]) + 0x1f) &
                ~0x1f;
        }
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
void AudioFxReverbStdNin::ReleaseWorkBuffer() {
    mIsBufferAssigned = false;
    for (u32 i = 0; i < cPairCountMax; i++) {
        mEarlyBuffer[i] = nullptr;
        mPreDelayBuffer[i] = nullptr;
    }
    for (u32 j = 0; j < cCombCount; j++) {
        for (u32 i = 0; i < cPairCountMax; i++) {
            mCombBuffer[j][i] = nullptr;
        }
    }
    for (u32 j = 0; j < cAllPassCount; j++) {
        for (u32 i = 0; i < cPairCountMax; i++) {
            mAllPassBuffer[j][i] = nullptr;
        }
    }
    for (u32 i = 0; i < 6; i++) {
        _1b8[i] = nullptr;
    }
    AudioFxBaseNin::ReleaseWorkBuffer();
}

/**
 * Rewinds the delay line positions.
 */
void AudioFxReverbStdNin::initBufferPos_() {
    mEarlyPos = 0;
    mPreDelayPos = 0;
    for (u32 i = 0; i < cCombCount; i++) {
        mCombPos[i] = 0;
    }
    for (u32 i = 0; i < cAllPassCount; i++) {
        mAllPassPos[i] = 0;
    }
    _1f0 = 0;
    _1f8 = 0;
}

/**
 * Gets the sample rate selected by the parameters.
 * @return Sample rate in Hz, or 0 for an unknown setting.
 */
f32 AudioFxReverbStdNin::getSampleRate_() const {
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

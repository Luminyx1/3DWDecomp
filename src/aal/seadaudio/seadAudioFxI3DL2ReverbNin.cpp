#include "audio/seadAudioFxI3DL2ReverbNin.h"

#include <cmath>
#include <cstring>

#include "audio/seadAudioSystemNin.h"
#include "audio/seadAudioVolumeUtil.h"
#include "math/seadMathCalcCommon.h"

namespace sead {
namespace {
s32 sI3DL2ReverbSamplesPerFrame = 0;

const u32 cI3DL2EarlyDelay32k[8] = {163, 317, 479, 641, 797, 967, 1123, 1283};
const u32 cI3DL2EarlyDelay48k[8] = {241, 479, 719, 967, 1193, 1451, 1693, 1931};

const f32 cI3DL2EarlyCoefLow = -0.33f;
const f32 cI3DL2EarlyCoefHigh = 0.33f;

const u32 cI3DL2FusedDelay32k[6][4] = {
    {1789, 1999, 433, 149}, {149, 293, 251, 103},   {947, 1361, 433, 137},
    {1279, 1531, 509, 149}, {1531, 1847, 563, 179}, {1823, 2357, 571, 137},
};

const u32 cI3DL2FusedDelay48k[6][4] = {
    {2683, 2999, 647, 223}, {223, 439, 379, 157},   {1423, 2039, 647, 211},
    {1913, 2297, 761, 223}, {2297, 2777, 839, 269}, {2731, 3539, 857, 211},
};
}  // namespace

/**
 * Constructs a parameter set with the default values.
 */
AudioFxI3DL2ReverbParamNin::AudioFxI3DL2ReverbParamNin()
    : mRoom(-1000), mRoomHf(0), mDecayTime(1.0f), mDecayHfRatio(0.5f), mReflections(-1000),
      mReflectionsDelay(0.02f), mReverb(-1000), mReverbDelay(0.04f), mDensity(100.0f),
      mDiffusion(100.0f), mHfReference(5000.0f), mEarlyMode(cEarlyMode_5), mFusedMode(0),
      mChannelCount(6), mSampleRate(0), _44(false), _48(8), _4c(4) {
    if (sI3DL2ReverbSamplesPerFrame == 0) {
        sI3DL2ReverbSamplesPerFrame = AudioSystemNin::GetSamplesPerFrame();
    }
}

/**
 * Constructs the effect.
 */
AudioFxI3DL2ReverbNin::AudioFxI3DL2ReverbNin() {
    initVars_();
    SetAudioFrameCount(5);
}

/**
 * Releases the work buffer and clears the delay settings and gains.
 */
void AudioFxI3DL2ReverbNin::initVars_() {
    ReleaseWorkBuffer();
    mReflectionsDelaySize = 0;
    mReflectionsPos = 0;
    mEarlyDelaySize = 0;
    mEarlyPos = 0;
    mEarlyCoef[0] = 0.0f;
    mEarlyCoef[1] = 0.0f;
    mReverbDelaySize = 0;
    mReverbDelayPos = 0;
    for (u32 j = 0; j < cCombCount; j++) {
        mCombDelaySize[j] = 0;
        mCombPos[j] = 0;
        mCombCoef[j][0] = 0.0f;
        mCombCoef[j][1] = 0.0f;
        for (u32 i = 0; i < cPairCountMax; i++) {
            mCombLpfHistory[j][i].set(0.0f, 0.0f);
        }
    }

    mCombLpfInGain[0] = 0.0f;
    mCombLpfInGain[1] = 0.0f;
    mCombLpfHistoryGain[0] = 0.0f;
    mCombLpfHistoryGain[1] = 0.0f;
    for (u32 j = 0; j < cAllPassCount; j++) {
        mAllPassDelaySize[j] = 0;
        mAllPassPos[j] = 0;
    }

    mAllPassCoef[0] = 0.0f;
    mAllPassCoef[1] = 0.0f;
    for (u32 i = 0; i < cPairCountMax; i++) {
        mLpfHistory[i].set(0.0f, 0.0f);
    }

    mLpfInGain[0] = 0.0f;
    mLpfInGain[1] = 0.0f;
    mLpfHistoryGain[0] = 0.0f;
    mLpfHistoryGain[1] = 0.0f;
    mEarlyGain[0] = 0.0f;
    mEarlyGain[1] = 0.0f;
    mReverbGain[0] = 0.0f;
    mReverbGain[1] = 0.0f;
}

/**
 * Finalizes and destroys the effect.
 */
AudioFxI3DL2ReverbNin::~AudioFxI3DL2ReverbNin() {
    Finalize();
}

/**
 * Clears the delay lines and starts the effect.
 * @return False if the effect was already initialized.
 */
bool AudioFxI3DL2ReverbNin::Initialize() {
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
void AudioFxI3DL2ReverbNin::clearBuffer_() {
    u32 pairCount = mChannelCountMax / 2;
    for (u32 i = 0; i < pairCount; i++) {
        if (mReflectionsBuffer[i]) {
            memset(mReflectionsBuffer[i], 0, mReflectionsDelaySize * sizeof(Vector2f));
        }

        memset(mEarlyBuffer[i], 0, mEarlyDelaySize * sizeof(Vector2f));
        if (mReverbDelayBuffer[i]) {
            memset(mReverbDelayBuffer[i], 0, mReverbDelaySize * sizeof(Vector2f));
        }
    }

    for (u32 j = 0; j < cCombCount; j++) {
        for (u32 i = 0; i < pairCount; i++) {
            memset(mCombBuffer[j][i], 0, mCombDelaySize[j] * sizeof(Vector2f));
            mCombLpfHistory[j][i].set(0.0f, 0.0f);
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
void AudioFxI3DL2ReverbNin::UpdateSamples(s32* pSamples, const UpdateSamplesArg& rArg) {
    if (!mIsInitialized) {
        return;
    }

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
void AudioFxI3DL2ReverbNin::updateFx2ch_(s32* pCh0, s32* pCh1, u32 sampleCount) {
    const f32 earlyCoef0 = mEarlyCoef[0];
    const f32 earlyCoef1 = mEarlyCoef[1];
    const f32 earlyGain0 = mEarlyGain[0];
    const f32 earlyGain1 = mEarlyGain[1];
    f32 combCoef[cCombCount][2];
    memcpy(combCoef, mCombCoef, sizeof(combCoef));
    const f32 combLpfInGain0 = mCombLpfInGain[0];
    const f32 combLpfInGain1 = mCombLpfInGain[1];
    const f32 combLpfHistoryGain0 = mCombLpfHistoryGain[0];
    const f32 combLpfHistoryGain1 = mCombLpfHistoryGain[1];
    const f32 allPassCoef0 = mAllPassCoef[0];
    const f32 allPassCoef1 = mAllPassCoef[1];
    const f32 lpfInGain0 = mLpfInGain[0];
    const f32 lpfInGain1 = mLpfInGain[1];
    const f32 lpfHistoryGain0 = mLpfHistoryGain[0];
    const f32 lpfHistoryGain1 = mLpfHistoryGain[1];
    const f32 reverbGain0 = mReverbGain[0];
    const f32 reverbGain1 = mReverbGain[1];
    for (u32 i = 0; i < sampleCount; i++) {
        Vector2f in0(*pCh0, *pCh1);
        if (mReflectionsDelaySize != 0) {
            Vector2f* reflections0 = &mReflectionsBuffer[0][mReflectionsPos];
            Vector2f delayed0 = *reflections0;
            *reflections0 = in0;
            in0 = delayed0;
        }

        Vector2f* early0 = &mEarlyBuffer[0][mEarlyPos];
        f32 early0x = early0->x;
        f32 early0y = early0->y;
        early0->set(in0.x + earlyCoef0 * early0x, in0.y + earlyCoef1 * early0y);
        f32 out0x = earlyGain0 * early0x;
        f32 out0y = earlyGain1 * early0y;
        Vector2f combIn0 = in0;
        if (mReverbDelaySize != 0) {
            Vector2f* reverbDelay0 = &mReverbDelayBuffer[0][mReverbDelayPos];
            combIn0 = *reverbDelay0;
            *reverbDelay0 = in0;
        }

        Vector2f* comb00 = &mCombBuffer[0][0][mCombPos[0]];
        f32 comb00x = comb00->x;
        f32 comb00y = comb00->y;
        f32 combLpf00x = combLpfInGain0 * comb00x + combLpfHistoryGain0 * mCombLpfHistory[0][0].x;
        f32 combLpf00y = combLpfInGain1 * comb00y + combLpfHistoryGain1 * mCombLpfHistory[0][0].y;
        mCombLpfHistory[0][0].x = combLpf00x;
        mCombLpfHistory[0][0].y = combLpf00y;
        comb00->set(combLpf00x * combCoef[0][0] + combIn0.x, combLpf00y * combCoef[0][1] + combIn0.y);
        Vector2f* comb10 = &mCombBuffer[1][0][mCombPos[1]];
        f32 comb10x = comb10->x;
        f32 comb10y = comb10->y;
        f32 combLpf10x = combLpfInGain0 * comb10x + combLpfHistoryGain0 * mCombLpfHistory[1][0].x;
        f32 combLpf10y = combLpfInGain1 * comb10y + combLpfHistoryGain1 * mCombLpfHistory[1][0].y;
        mCombLpfHistory[1][0].x = combLpf10x;
        mCombLpfHistory[1][0].y = combLpf10y;
        comb10->set(combLpf10x * combCoef[1][0] + combIn0.x, combLpf10y * combCoef[1][1] + combIn0.y);
        f32 wet0x = comb00x + comb10x;
        f32 wet0y = comb00y + comb10y;
        Vector2f* allPass00 = &mAllPassBuffer[0][0][mAllPassPos[0]];
        f32 allPass00x = allPass00->x;
        f32 allPass00y = allPass00->y;
        f32 temp00x = wet0x + allPassCoef0 * allPass00x;
        f32 temp00y = wet0y + allPassCoef1 * allPass00y;
        allPass00->set(temp00x, temp00y);
        wet0x = allPassCoef0 * temp00x - allPass00x;
        wet0y = allPassCoef1 * temp00y - allPass00y;
        Vector2f* allPass10 = &mAllPassBuffer[1][0][mAllPassPos[1]];
        f32 allPass10x = allPass10->x;
        f32 allPass10y = allPass10->y;
        f32 temp10x = wet0x + allPassCoef0 * allPass10x;
        f32 temp10y = wet0y + allPassCoef1 * allPass10y;
        allPass10->set(temp10x, temp10y);
        wet0x = allPassCoef0 * temp10x - allPass10x;
        wet0y = allPassCoef1 * temp10y - allPass10y;
        out0x += reverbGain0 * wet0x;
        out0y += reverbGain1 * wet0y;
        f32 lpf0x = lpfHistoryGain0 * mLpfHistory[0].x + lpfInGain0 * out0x;
        f32 lpf0y = lpfHistoryGain1 * mLpfHistory[0].y + lpfInGain1 * out0y;
        mLpfHistory[0].x = lpf0x;
        mLpfHistory[0].y = lpf0y;
        *pCh0++ = lpf0x;
        *pCh1++ = lpf0y;

        if (mReflectionsDelaySize != 0) {
            mReflectionsPos = mReflectionsPos + 1 >= mReflectionsDelaySize ? 0 : mReflectionsPos + 1;
        }

        mEarlyPos = mEarlyPos + 1 >= mEarlyDelaySize ? 0 : mEarlyPos + 1;
        if (mReverbDelaySize != 0) {
            mReverbDelayPos = mReverbDelayPos + 1 >= mReverbDelaySize ? 0 : mReverbDelayPos + 1;
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
void AudioFxI3DL2ReverbNin::updateFx4ch_(s32* pCh0, s32* pCh1, s32* pCh2, s32* pCh3, u32 sampleCount) {
    const f32 earlyCoef0 = mEarlyCoef[0];
    const f32 earlyCoef1 = mEarlyCoef[1];
    const f32 earlyGain0 = mEarlyGain[0];
    const f32 earlyGain1 = mEarlyGain[1];
    f32 combCoef[cCombCount][2];
    memcpy(combCoef, mCombCoef, sizeof(combCoef));
    const f32 combLpfInGain0 = mCombLpfInGain[0];
    const f32 combLpfInGain1 = mCombLpfInGain[1];
    const f32 combLpfHistoryGain0 = mCombLpfHistoryGain[0];
    const f32 combLpfHistoryGain1 = mCombLpfHistoryGain[1];
    const f32 allPassCoef0 = mAllPassCoef[0];
    const f32 allPassCoef1 = mAllPassCoef[1];
    const f32 lpfInGain0 = mLpfInGain[0];
    const f32 lpfInGain1 = mLpfInGain[1];
    const f32 lpfHistoryGain0 = mLpfHistoryGain[0];
    const f32 lpfHistoryGain1 = mLpfHistoryGain[1];
    const f32 reverbGain0 = mReverbGain[0];
    const f32 reverbGain1 = mReverbGain[1];
    for (u32 i = 0; i < sampleCount; i++) {
        Vector2f in0(*pCh0, *pCh1);
        Vector2f in1(*pCh2, *pCh3);
        if (mReflectionsDelaySize != 0) {
            Vector2f* reflections0 = &mReflectionsBuffer[0][mReflectionsPos];
            Vector2f delayed0 = *reflections0;
            *reflections0 = in0;
            in0 = delayed0;
            Vector2f* reflections1 = &mReflectionsBuffer[1][mReflectionsPos];
            Vector2f delayed1 = *reflections1;
            *reflections1 = in1;
            in1 = delayed1;
        }

        Vector2f* early0 = &mEarlyBuffer[0][mEarlyPos];
        f32 early0x = early0->x;
        f32 early0y = early0->y;
        early0->set(in0.x + earlyCoef0 * early0x, in0.y + earlyCoef1 * early0y);
        f32 out0x = earlyGain0 * early0x;
        f32 out0y = earlyGain1 * early0y;
        Vector2f* early1 = &mEarlyBuffer[1][mEarlyPos];
        f32 early1x = early1->x;
        f32 early1y = early1->y;
        early1->set(in1.x + earlyCoef0 * early1x, in1.y + earlyCoef1 * early1y);
        f32 out1x = earlyGain0 * early1x;
        f32 out1y = earlyGain1 * early1y;
        Vector2f combIn0 = in0;
        Vector2f combIn1 = in1;
        if (mReverbDelaySize != 0) {
            Vector2f* reverbDelay0 = &mReverbDelayBuffer[0][mReverbDelayPos];
            combIn0 = *reverbDelay0;
            *reverbDelay0 = in0;
            Vector2f* reverbDelay1 = &mReverbDelayBuffer[1][mReverbDelayPos];
            combIn1 = *reverbDelay1;
            *reverbDelay1 = in1;
        }

        Vector2f* comb00 = &mCombBuffer[0][0][mCombPos[0]];
        f32 comb00x = comb00->x;
        f32 comb00y = comb00->y;
        f32 combLpf00x = combLpfInGain0 * comb00x + combLpfHistoryGain0 * mCombLpfHistory[0][0].x;
        f32 combLpf00y = combLpfInGain1 * comb00y + combLpfHistoryGain1 * mCombLpfHistory[0][0].y;
        mCombLpfHistory[0][0].x = combLpf00x;
        mCombLpfHistory[0][0].y = combLpf00y;
        comb00->set(combLpf00x * combCoef[0][0] + combIn0.x, combLpf00y * combCoef[0][1] + combIn0.y);
        Vector2f* comb01 = &mCombBuffer[0][1][mCombPos[0]];
        f32 comb01x = comb01->x;
        f32 comb01y = comb01->y;
        f32 combLpf01x = combLpfInGain0 * comb01x + combLpfHistoryGain0 * mCombLpfHistory[0][1].x;
        f32 combLpf01y = combLpfInGain1 * comb01y + combLpfHistoryGain1 * mCombLpfHistory[0][1].y;
        mCombLpfHistory[0][1].x = combLpf01x;
        mCombLpfHistory[0][1].y = combLpf01y;
        comb01->set(combLpf01x * combCoef[0][0] + combIn1.x, combLpf01y * combCoef[0][1] + combIn1.y);
        Vector2f* comb10 = &mCombBuffer[1][0][mCombPos[1]];
        f32 comb10x = comb10->x;
        f32 comb10y = comb10->y;
        f32 combLpf10x = combLpfInGain0 * comb10x + combLpfHistoryGain0 * mCombLpfHistory[1][0].x;
        f32 combLpf10y = combLpfInGain1 * comb10y + combLpfHistoryGain1 * mCombLpfHistory[1][0].y;
        mCombLpfHistory[1][0].x = combLpf10x;
        mCombLpfHistory[1][0].y = combLpf10y;
        comb10->set(combLpf10x * combCoef[1][0] + combIn0.x, combLpf10y * combCoef[1][1] + combIn0.y);
        Vector2f* comb11 = &mCombBuffer[1][1][mCombPos[1]];
        f32 comb11x = comb11->x;
        f32 comb11y = comb11->y;
        f32 combLpf11x = combLpfInGain0 * comb11x + combLpfHistoryGain0 * mCombLpfHistory[1][1].x;
        f32 combLpf11y = combLpfInGain1 * comb11y + combLpfHistoryGain1 * mCombLpfHistory[1][1].y;
        mCombLpfHistory[1][1].x = combLpf11x;
        mCombLpfHistory[1][1].y = combLpf11y;
        comb11->set(combLpf11x * combCoef[1][0] + combIn1.x, combLpf11y * combCoef[1][1] + combIn1.y);
        f32 wet0x = comb00x + comb10x;
        f32 wet0y = comb00y + comb10y;
        f32 wet1x = comb01x + comb11x;
        f32 wet1y = comb01y + comb11y;
        Vector2f* allPass00 = &mAllPassBuffer[0][0][mAllPassPos[0]];
        f32 allPass00x = allPass00->x;
        f32 allPass00y = allPass00->y;
        f32 temp00x = wet0x + allPassCoef0 * allPass00x;
        f32 temp00y = wet0y + allPassCoef1 * allPass00y;
        allPass00->set(temp00x, temp00y);
        wet0x = allPassCoef0 * temp00x - allPass00x;
        wet0y = allPassCoef1 * temp00y - allPass00y;
        Vector2f* allPass01 = &mAllPassBuffer[0][1][mAllPassPos[0]];
        f32 allPass01x = allPass01->x;
        f32 allPass01y = allPass01->y;
        f32 temp01x = wet1x + allPassCoef0 * allPass01x;
        f32 temp01y = wet1y + allPassCoef1 * allPass01y;
        allPass01->set(temp01x, temp01y);
        wet1x = allPassCoef0 * temp01x - allPass01x;
        wet1y = allPassCoef1 * temp01y - allPass01y;
        Vector2f* allPass10 = &mAllPassBuffer[1][0][mAllPassPos[1]];
        f32 allPass10x = allPass10->x;
        f32 allPass10y = allPass10->y;
        f32 temp10x = wet0x + allPassCoef0 * allPass10x;
        f32 temp10y = wet0y + allPassCoef1 * allPass10y;
        allPass10->set(temp10x, temp10y);
        wet0x = allPassCoef0 * temp10x - allPass10x;
        wet0y = allPassCoef1 * temp10y - allPass10y;
        Vector2f* allPass11 = &mAllPassBuffer[1][1][mAllPassPos[1]];
        f32 allPass11x = allPass11->x;
        f32 allPass11y = allPass11->y;
        f32 temp11x = wet1x + allPassCoef0 * allPass11x;
        f32 temp11y = wet1y + allPassCoef1 * allPass11y;
        allPass11->set(temp11x, temp11y);
        wet1x = allPassCoef0 * temp11x - allPass11x;
        wet1y = allPassCoef1 * temp11y - allPass11y;
        out0x += reverbGain0 * wet0x;
        out0y += reverbGain1 * wet0y;
        f32 lpf0x = lpfHistoryGain0 * mLpfHistory[0].x + lpfInGain0 * out0x;
        f32 lpf0y = lpfHistoryGain1 * mLpfHistory[0].y + lpfInGain1 * out0y;
        mLpfHistory[0].x = lpf0x;
        mLpfHistory[0].y = lpf0y;
        *pCh0++ = lpf0x;
        *pCh1++ = lpf0y;
        out1x += reverbGain0 * wet1x;
        out1y += reverbGain1 * wet1y;
        f32 lpf1x = lpfHistoryGain0 * mLpfHistory[1].x + lpfInGain0 * out1x;
        f32 lpf1y = lpfHistoryGain1 * mLpfHistory[1].y + lpfInGain1 * out1y;
        mLpfHistory[1].x = lpf1x;
        mLpfHistory[1].y = lpf1y;
        *pCh2++ = lpf1x;
        *pCh3++ = lpf1y;

        if (mReflectionsDelaySize != 0) {
            mReflectionsPos = mReflectionsPos + 1 >= mReflectionsDelaySize ? 0 : mReflectionsPos + 1;
        }

        mEarlyPos = mEarlyPos + 1 >= mEarlyDelaySize ? 0 : mEarlyPos + 1;
        if (mReverbDelaySize != 0) {
            mReverbDelayPos = mReverbDelayPos + 1 >= mReverbDelaySize ? 0 : mReverbDelayPos + 1;
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
void AudioFxI3DL2ReverbNin::updateFx6ch_(s32* pCh0, s32* pCh1, s32* pCh2, s32* pCh3, s32* pCh4, s32* pCh5, u32 sampleCount) {
    const f32 earlyCoef0 = mEarlyCoef[0];
    const f32 earlyCoef1 = mEarlyCoef[1];
    const f32 earlyGain0 = mEarlyGain[0];
    const f32 earlyGain1 = mEarlyGain[1];
    f32 combCoef[cCombCount][2];
    memcpy(combCoef, mCombCoef, sizeof(combCoef));
    const f32 combLpfInGain0 = mCombLpfInGain[0];
    const f32 combLpfInGain1 = mCombLpfInGain[1];
    const f32 combLpfHistoryGain0 = mCombLpfHistoryGain[0];
    const f32 combLpfHistoryGain1 = mCombLpfHistoryGain[1];
    const f32 allPassCoef0 = mAllPassCoef[0];
    const f32 allPassCoef1 = mAllPassCoef[1];
    const f32 lpfInGain0 = mLpfInGain[0];
    const f32 lpfInGain1 = mLpfInGain[1];
    const f32 lpfHistoryGain0 = mLpfHistoryGain[0];
    const f32 lpfHistoryGain1 = mLpfHistoryGain[1];
    const f32 reverbGain0 = mReverbGain[0];
    const f32 reverbGain1 = mReverbGain[1];
    for (u32 i = 0; i < sampleCount; i++) {
        Vector2f in0(*pCh0, *pCh1);
        Vector2f in1(*pCh2, *pCh3);
        Vector2f in2(*pCh4, *pCh5);
        if (mReflectionsDelaySize != 0) {
            Vector2f* reflections0 = &mReflectionsBuffer[0][mReflectionsPos];
            Vector2f delayed0 = *reflections0;
            *reflections0 = in0;
            in0 = delayed0;
            Vector2f* reflections1 = &mReflectionsBuffer[1][mReflectionsPos];
            Vector2f delayed1 = *reflections1;
            *reflections1 = in1;
            in1 = delayed1;
            Vector2f* reflections2 = &mReflectionsBuffer[2][mReflectionsPos];
            Vector2f delayed2 = *reflections2;
            *reflections2 = in2;
            in2 = delayed2;
        }

        Vector2f* early0 = &mEarlyBuffer[0][mEarlyPos];
        f32 early0x = early0->x;
        f32 early0y = early0->y;
        early0->set(in0.x + earlyCoef0 * early0x, in0.y + earlyCoef1 * early0y);
        f32 out0x = earlyGain0 * early0x;
        f32 out0y = earlyGain1 * early0y;
        Vector2f* early1 = &mEarlyBuffer[1][mEarlyPos];
        f32 early1x = early1->x;
        f32 early1y = early1->y;
        early1->set(in1.x + earlyCoef0 * early1x, in1.y + earlyCoef1 * early1y);
        f32 out1x = earlyGain0 * early1x;
        f32 out1y = earlyGain1 * early1y;
        Vector2f* early2 = &mEarlyBuffer[2][mEarlyPos];
        f32 early2x = early2->x;
        f32 early2y = early2->y;
        early2->set(in2.x + earlyCoef0 * early2x, in2.y + earlyCoef1 * early2y);
        f32 out2x = earlyGain0 * early2x;
        f32 out2y = earlyGain1 * early2y;
        Vector2f combIn0 = in0;
        Vector2f combIn1 = in1;
        Vector2f combIn2 = in2;
        if (mReverbDelaySize != 0) {
            Vector2f* reverbDelay0 = &mReverbDelayBuffer[0][mReverbDelayPos];
            combIn0 = *reverbDelay0;
            *reverbDelay0 = in0;
            Vector2f* reverbDelay1 = &mReverbDelayBuffer[1][mReverbDelayPos];
            combIn1 = *reverbDelay1;
            *reverbDelay1 = in1;
            Vector2f* reverbDelay2 = &mReverbDelayBuffer[2][mReverbDelayPos];
            combIn2 = *reverbDelay2;
            *reverbDelay2 = in2;
        }

        Vector2f* comb00 = &mCombBuffer[0][0][mCombPos[0]];
        f32 comb00x = comb00->x;
        f32 comb00y = comb00->y;
        f32 combLpf00x = combLpfInGain0 * comb00x + combLpfHistoryGain0 * mCombLpfHistory[0][0].x;
        f32 combLpf00y = combLpfInGain1 * comb00y + combLpfHistoryGain1 * mCombLpfHistory[0][0].y;
        mCombLpfHistory[0][0].x = combLpf00x;
        mCombLpfHistory[0][0].y = combLpf00y;
        comb00->set(combLpf00x * combCoef[0][0] + combIn0.x, combLpf00y * combCoef[0][1] + combIn0.y);
        Vector2f* comb01 = &mCombBuffer[0][1][mCombPos[0]];
        f32 comb01x = comb01->x;
        f32 comb01y = comb01->y;
        f32 combLpf01x = combLpfInGain0 * comb01x + combLpfHistoryGain0 * mCombLpfHistory[0][1].x;
        f32 combLpf01y = combLpfInGain1 * comb01y + combLpfHistoryGain1 * mCombLpfHistory[0][1].y;
        mCombLpfHistory[0][1].x = combLpf01x;
        mCombLpfHistory[0][1].y = combLpf01y;
        comb01->set(combLpf01x * combCoef[0][0] + combIn1.x, combLpf01y * combCoef[0][1] + combIn1.y);
        Vector2f* comb02 = &mCombBuffer[0][2][mCombPos[0]];
        f32 comb02x = comb02->x;
        f32 comb02y = comb02->y;
        f32 combLpf02x = combLpfInGain0 * comb02x + combLpfHistoryGain0 * mCombLpfHistory[0][2].x;
        f32 combLpf02y = combLpfInGain1 * comb02y + combLpfHistoryGain1 * mCombLpfHistory[0][2].y;
        mCombLpfHistory[0][2].x = combLpf02x;
        mCombLpfHistory[0][2].y = combLpf02y;
        comb02->set(combLpf02x * combCoef[0][0] + combIn2.x, combLpf02y * combCoef[0][1] + combIn2.y);
        Vector2f* comb10 = &mCombBuffer[1][0][mCombPos[1]];
        f32 comb10x = comb10->x;
        f32 comb10y = comb10->y;
        f32 combLpf10x = combLpfInGain0 * comb10x + combLpfHistoryGain0 * mCombLpfHistory[1][0].x;
        f32 combLpf10y = combLpfInGain1 * comb10y + combLpfHistoryGain1 * mCombLpfHistory[1][0].y;
        mCombLpfHistory[1][0].x = combLpf10x;
        mCombLpfHistory[1][0].y = combLpf10y;
        comb10->set(combLpf10x * combCoef[1][0] + combIn0.x, combLpf10y * combCoef[1][1] + combIn0.y);
        Vector2f* comb11 = &mCombBuffer[1][1][mCombPos[1]];
        f32 comb11x = comb11->x;
        f32 comb11y = comb11->y;
        f32 combLpf11x = combLpfInGain0 * comb11x + combLpfHistoryGain0 * mCombLpfHistory[1][1].x;
        f32 combLpf11y = combLpfInGain1 * comb11y + combLpfHistoryGain1 * mCombLpfHistory[1][1].y;
        mCombLpfHistory[1][1].x = combLpf11x;
        mCombLpfHistory[1][1].y = combLpf11y;
        comb11->set(combLpf11x * combCoef[1][0] + combIn1.x, combLpf11y * combCoef[1][1] + combIn1.y);
        Vector2f* comb12 = &mCombBuffer[1][2][mCombPos[1]];
        f32 comb12x = comb12->x;
        f32 comb12y = comb12->y;
        f32 combLpf12x = combLpfInGain0 * comb12x + combLpfHistoryGain0 * mCombLpfHistory[1][2].x;
        f32 combLpf12y = combLpfInGain1 * comb12y + combLpfHistoryGain1 * mCombLpfHistory[1][2].y;
        mCombLpfHistory[1][2].x = combLpf12x;
        mCombLpfHistory[1][2].y = combLpf12y;
        comb12->set(combLpf12x * combCoef[1][0] + combIn2.x, combLpf12y * combCoef[1][1] + combIn2.y);
        f32 wet0x = comb00x + comb10x;
        f32 wet0y = comb00y + comb10y;
        f32 wet1x = comb01x + comb11x;
        f32 wet1y = comb01y + comb11y;
        f32 wet2x = comb02x + comb12x;
        f32 wet2y = comb02y + comb12y;
        Vector2f* allPass00 = &mAllPassBuffer[0][0][mAllPassPos[0]];
        f32 allPass00x = allPass00->x;
        f32 allPass00y = allPass00->y;
        f32 temp00x = wet0x + allPassCoef0 * allPass00x;
        f32 temp00y = wet0y + allPassCoef1 * allPass00y;
        allPass00->set(temp00x, temp00y);
        wet0x = allPassCoef0 * temp00x - allPass00x;
        wet0y = allPassCoef1 * temp00y - allPass00y;
        Vector2f* allPass01 = &mAllPassBuffer[0][1][mAllPassPos[0]];
        f32 allPass01x = allPass01->x;
        f32 allPass01y = allPass01->y;
        f32 temp01x = wet1x + allPassCoef0 * allPass01x;
        f32 temp01y = wet1y + allPassCoef1 * allPass01y;
        allPass01->set(temp01x, temp01y);
        wet1x = allPassCoef0 * temp01x - allPass01x;
        wet1y = allPassCoef1 * temp01y - allPass01y;
        Vector2f* allPass02 = &mAllPassBuffer[0][2][mAllPassPos[0]];
        f32 allPass02x = allPass02->x;
        f32 allPass02y = allPass02->y;
        f32 temp02x = wet2x + allPassCoef0 * allPass02x;
        f32 temp02y = wet2y + allPassCoef1 * allPass02y;
        allPass02->set(temp02x, temp02y);
        wet2x = allPassCoef0 * temp02x - allPass02x;
        wet2y = allPassCoef1 * temp02y - allPass02y;
        Vector2f* allPass10 = &mAllPassBuffer[1][0][mAllPassPos[1]];
        f32 allPass10x = allPass10->x;
        f32 allPass10y = allPass10->y;
        f32 temp10x = wet0x + allPassCoef0 * allPass10x;
        f32 temp10y = wet0y + allPassCoef1 * allPass10y;
        allPass10->set(temp10x, temp10y);
        wet0x = allPassCoef0 * temp10x - allPass10x;
        wet0y = allPassCoef1 * temp10y - allPass10y;
        Vector2f* allPass11 = &mAllPassBuffer[1][1][mAllPassPos[1]];
        f32 allPass11x = allPass11->x;
        f32 allPass11y = allPass11->y;
        f32 temp11x = wet1x + allPassCoef0 * allPass11x;
        f32 temp11y = wet1y + allPassCoef1 * allPass11y;
        allPass11->set(temp11x, temp11y);
        wet1x = allPassCoef0 * temp11x - allPass11x;
        wet1y = allPassCoef1 * temp11y - allPass11y;
        Vector2f* allPass12 = &mAllPassBuffer[1][2][mAllPassPos[1]];
        f32 allPass12x = allPass12->x;
        f32 allPass12y = allPass12->y;
        f32 temp12x = wet2x + allPassCoef0 * allPass12x;
        f32 temp12y = wet2y + allPassCoef1 * allPass12y;
        allPass12->set(temp12x, temp12y);
        wet2x = allPassCoef0 * temp12x - allPass12x;
        wet2y = allPassCoef1 * temp12y - allPass12y;
        out0x += reverbGain0 * wet0x;
        out0y += reverbGain1 * wet0y;
        f32 lpf0x = lpfHistoryGain0 * mLpfHistory[0].x + lpfInGain0 * out0x;
        f32 lpf0y = lpfHistoryGain1 * mLpfHistory[0].y + lpfInGain1 * out0y;
        mLpfHistory[0].x = lpf0x;
        mLpfHistory[0].y = lpf0y;
        *pCh0++ = lpf0x;
        *pCh1++ = lpf0y;
        out1x += reverbGain0 * wet1x;
        out1y += reverbGain1 * wet1y;
        f32 lpf1x = lpfHistoryGain0 * mLpfHistory[1].x + lpfInGain0 * out1x;
        f32 lpf1y = lpfHistoryGain1 * mLpfHistory[1].y + lpfInGain1 * out1y;
        mLpfHistory[1].x = lpf1x;
        mLpfHistory[1].y = lpf1y;
        *pCh2++ = lpf1x;
        *pCh3++ = lpf1y;
        out2x += reverbGain0 * wet2x;
        out2y += reverbGain1 * wet2y;
        f32 lpf2x = lpfHistoryGain0 * mLpfHistory[2].x + lpfInGain0 * out2x;
        f32 lpf2y = lpfHistoryGain1 * mLpfHistory[2].y + lpfInGain1 * out2y;
        mLpfHistory[2].x = lpf2x;
        mLpfHistory[2].y = lpf2y;
        *pCh4++ = lpf2x;
        *pCh5++ = lpf2y;

        if (mReflectionsDelaySize != 0) {
            mReflectionsPos = mReflectionsPos + 1 >= mReflectionsDelaySize ? 0 : mReflectionsPos + 1;
        }

        mEarlyPos = mEarlyPos + 1 >= mEarlyDelaySize ? 0 : mEarlyPos + 1;
        if (mReverbDelaySize != 0) {
            mReverbDelayPos = mReverbDelayPos + 1 >= mReverbDelaySize ? 0 : mReverbDelayPos + 1;
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
void AudioFxI3DL2ReverbNin::Finalize() {
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
bool AudioFxI3DL2ReverbNin::SetParam(const AudioFxI3DL2ReverbParamNin& rParam) {
    mChannelCountMax = rParam.mChannelCount;
    if (!mIsBufferAssigned) {
        setupDelaySizes_(rParam);
    }

    setupGains_(rParam);
    _248 = rParam._48;
    _24c = rParam._4c;
    return true;
}

/**
 * Computes the delay line lengths from the parameters and sample rate.
 * @param rParam Reverb parameters.
 */
void AudioFxI3DL2ReverbNin::setupDelaySizes_(const AudioFxI3DL2ReverbParamNin& rParam) {
    mSampleRate = rParam.mSampleRate;
    mReflectionsDelaySize = rParam.mReflectionsDelay * getSampleRate_();
    const u32* earlyTable = mSampleRate == 0 ? cI3DL2EarlyDelay32k : cI3DL2EarlyDelay48k;
    mEarlyDelaySize = earlyTable[rParam.mEarlyMode];
    const f32& earlyCoef = rParam.mEarlyMode < AudioFxI3DL2ReverbParamNin::cEarlyMode_4 ?
                               cI3DL2EarlyCoefLow :
                               cI3DL2EarlyCoefHigh;
    mEarlyCoef[0] = earlyCoef;
    mEarlyCoef[1] = earlyCoef;
    mReverbDelaySize = getSampleRate_() * rParam.mReverbDelay;
    const u32(*fusedTable)[4] = nullptr;
    switch (mSampleRate) {
    case 0:
        fusedTable = cI3DL2FusedDelay32k;
        break;
    case 1:
        fusedTable = cI3DL2FusedDelay48k;
        break;
    }

    const u32* fused = fusedTable[rParam.mFusedMode];
    mCombDelaySize[0] = (2.0f - rParam.mDensity / 100.0f) * fused[0];
    mCombDelaySize[1] = (2.0f - rParam.mDensity / 100.0f) * fused[1];
    mAllPassDelaySize[0] = fused[2];
    mAllPassDelaySize[1] = fused[3];
}

/**
 * Computes the filter coefficients and gains from the parameters.
 * @param rParam Reverb parameters.
 */
void AudioFxI3DL2ReverbNin::setupGains_(const AudioFxI3DL2ReverbParamNin& rParam) {
    f32 room = AudioVolumeUtil::calcVolumeRatioFromMillibelTable(rParam.mRoom);
    mEarlyGain[0] = room * AudioVolumeUtil::calcVolumeRatioFromMillibelTable(rParam.mReflections);
    mEarlyGain[1] = mEarlyGain[0];
    f32 hfReference = rParam.mHfReference;
    for (u32 i = 0; i < cCombCount; i++) {
        mCombCoef[i][0] = std::pow(
            10.0f, (mCombDelaySize[i] * -3.0f) / (rParam.mDecayTime * getSampleRate_()));
        mCombCoef[i][1] = mCombCoef[i][0];
    }

    f32 combLpfCoef =
        calcLpfCoefWithGain_(hfReference, rParam.mDecayHfRatio * 0.9f + 0.1f);
    mCombLpfInGain[0] = 1.0f - combLpfCoef;
    mCombLpfInGain[1] = mCombLpfInGain[0];
    mCombLpfHistoryGain[0] = combLpfCoef;
    mCombLpfHistoryGain[1] = combLpfCoef;
    mAllPassCoef[0] = rParam.mDiffusion / -100.0f + 1.0f;
    mAllPassCoef[1] = mAllPassCoef[0];
    f32 lpfCoef = calcLpfCoef_(hfReference, rParam.mRoomHf);
    mLpfInGain[0] = 1.0f - lpfCoef;
    mLpfInGain[1] = mLpfInGain[0];
    mLpfHistoryGain[0] = lpfCoef;
    mLpfHistoryGain[1] = lpfCoef;
    mReverbGain[0] = room * AudioVolumeUtil::calcVolumeRatioFromMillibelTable(rParam.mReverb);
    mReverbGain[1] = mReverbGain[0];
}

/**
 * Gets the work buffer size needed for the aux buffer and all delay lines.
 * @return Required work buffer size in bytes.
 */
size_t AudioFxI3DL2ReverbNin::GetRequiredMemSize() const {
    u32 pairCount = mChannelCountMax / 2;
    u32 delaySize = ((mReflectionsDelaySize * sizeof(Vector2f) + 0x1f) & ~0x1f) +
                    ((mEarlyDelaySize * sizeof(Vector2f) + 0x1f) & ~0x1f) +
                    ((mReverbDelaySize * sizeof(Vector2f) + 0x1f) & ~0x1f);
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
bool AudioFxI3DL2ReverbNin::AssignWorkBuffer(void* pBuffer, u32 size) {
    if (mIsBufferAssigned) {
        return false;
    }

    AudioFxBaseNin::AssignWorkBuffer(pBuffer, size);
    uintptr_t start = reinterpret_cast<uintptr_t>(mFxWorkBuffer);
    u32 pairCount = mChannelCountMax / 2;
    uintptr_t current = (start + 0x1f) & ~0x1f;
    if (mReflectionsDelaySize != 0) {
        for (u32 i = 0; i < pairCount; i++) {
            mReflectionsBuffer[i] = reinterpret_cast<Vector2f*>(current);
            current = (reinterpret_cast<uintptr_t>(mReflectionsBuffer[i] + mReflectionsDelaySize) +
                       0x1f) &
                      ~0x1f;
        }
    }

    for (u32 i = 0; i < pairCount; i++) {
        mEarlyBuffer[i] = reinterpret_cast<Vector2f*>(current);
        current = (reinterpret_cast<uintptr_t>(mEarlyBuffer[i] + mEarlyDelaySize) + 0x1f) & ~0x1f;
    }

    if (mReverbDelaySize != 0) {
        for (u32 i = 0; i < pairCount; i++) {
            mReverbDelayBuffer[i] = reinterpret_cast<Vector2f*>(current);
            current =
                (reinterpret_cast<uintptr_t>(mReverbDelayBuffer[i] + mReverbDelaySize) + 0x1f) &
                ~0x1f;
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
void AudioFxI3DL2ReverbNin::ReleaseWorkBuffer() {
    mIsBufferAssigned = false;
    for (u32 i = 0; i < cPairCountMax; i++) {
        mReflectionsBuffer[i] = nullptr;
        mEarlyBuffer[i] = nullptr;
        mReverbDelayBuffer[i] = nullptr;
    }

    for (u32 i = 0; i < 6; i++) {
        _218[i] = nullptr;
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

    AudioFxBaseNin::ReleaseWorkBuffer();
}

/**
 * Rewinds the delay line positions.
 */
void AudioFxI3DL2ReverbNin::initBufferPos_() {
    mReflectionsPos = 0;
    mEarlyPos = 0;
    mReverbDelayPos = 0;
    for (u32 i = 0; i < cCombCount; i++) {
        mCombPos[i] = 0;
    }

    for (u32 i = 0; i < cAllPassCount; i++) {
        mAllPassPos[i] = 0;
    }

    _258 = 0;
    _250 = 0;
}

/**
 * Gets the sample rate selected by the parameters.
 * @return Sample rate in Hz, or 0 for an unknown setting.
 */
f32 AudioFxI3DL2ReverbNin::getSampleRate_() const {
    switch (mSampleRate) {
    case 0:
        return 32000.0f;
    case 1:
        return 48000.0f;
    default:
        return 0.0f;
    }
}

/**
 * Converts a density percentage to a comb delay length factor.
 * @param density Density in percent.
 * @return Delay length factor.
 */
f32 AudioFxI3DL2ReverbNin::calcCombDelaySizeRate_(f32 density) {
    return density / -100.0f + 2.0f;
}

/**
 * Computes a one-pole low-pass coefficient for a gain at a frequency.
 * @param frequency Reference frequency in Hz.
 * @param gain Linear gain at the frequency.
 * @return Filter coefficient, 0 for unity gain.
 */
f32 AudioFxI3DL2ReverbNin::calcLpfCoefWithGain_(f32 frequency, f32 gain) {
    if (gain == 1.0f) {
        return 0.0f;
    }

    f32 c = std::cos(frequency * 6.2831855f / getSampleRate_());
    f32 a = 1.0f - c * gain;
    f32 b = (gain + gain) * (1.0f - c) - gain * gain * (1.0f - c * c);
    return (a - Mathf::sqrt(b)) / (1.0f - gain);
}

/**
 * Computes a one-pole low-pass coefficient for a millibel attenuation at a frequency.
 * @param frequency Reference frequency in Hz.
 * @param millibel Attenuation in millibels.
 * @return Filter coefficient, 0 for no attenuation.
 */
f32 AudioFxI3DL2ReverbNin::calcLpfCoef_(f32 frequency, f32 millibel) {
    if (millibel == 0.0f) {
        return 0.0f;
    }

    f32 gain = Mathf::expTable(millibel * 0.001f * Mathf::logTable(10.0f));
    return calcLpfCoefWithGain_(frequency, gain);
}
}  // namespace sead

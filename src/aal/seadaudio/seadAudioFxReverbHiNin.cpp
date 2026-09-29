#include "audio/seadAudioFxReverbHiNin.h"

#include <cmath>
#include <cstring>

#include "audio/seadAudioSystemNin.h"
#include "math/seadMathCalcCommon.h"

namespace sead {
namespace {
s32 sReverbHiSamplesPerFrame = 0;

const u32 cReverbHiEarlyDelay32k[8][3] = {
    {157, 479, 829},   {317, 809, 1117},  {479, 941, 1487},  {641, 1259, 1949},
    {797, 1667, 2579}, {967, 1901, 2903}, {1123, 2179, 3413}, {1279, 2477, 3889},
};

const u32 cReverbHiEarlyDelay48k[8][3] = {
    {239, 719, 1249},   {479, 1213, 1669},  {719, 1423, 2237},  {967, 1889, 2927},
    {1193, 2503, 3877}, {1451, 2851, 4257}, {1693, 3271, 5119}, {1931, 3719, 5839},
};

const u32 cReverbHiFusedDelay32k[6][7] = {
    {1789, 1999, 2333, 433, 149, 47, 73},  {149, 293, 449, 251, 103, 47, 73},
    {947, 1361, 1531, 433, 137, 47, 73},   {1279, 1531, 1973, 509, 149, 47, 73},
    {1531, 1847, 2297, 563, 179, 47, 73},  {1823, 2357, 2693, 571, 137, 47, 73},
};

const u32 cReverbHiFusedDelay48k[6][7] = {
    {2683, 2999, 3499, 647, 223, 71, 109}, {223, 439, 661, 379, 157, 71, 109},
    {1423, 2039, 2297, 647, 211, 71, 109}, {1913, 2297, 2957, 761, 223, 71, 109},
    {2297, 2777, 3449, 839, 269, 71, 109}, {2731, 3539, 4049, 857, 211, 71, 109},
};

const f32 cReverbHiEarlyGain[8][3] = {
    {0.4f, -1.0f, 0.3f},  {0.5f, -0.95f, 0.3f}, {0.6f, -0.9f, 0.3f}, {0.75f, -0.85f, 0.3f},
    {-0.9f, 0.8f, 0.3f},  {-1.0f, 0.7f, 0.3f},  {-1.0f, 0.7f, 0.3f}, {-1.0f, 0.7f, 0.3f},
};
}  // namespace

/**
 * Constructs a parameter set with the default values.
 */
AudioFxReverbHiParamNin::AudioFxReverbHiParamNin()
    : mPreDelayTime(0.02f), mDecayTime(3.0f), mColoration(0.6f), mLpfAmount(0.4f),
      mCrossTalk(0.1f), mOutGain(1.0f), mEarlyMode(5), mFusedMode(0), mEarlyGain(0.0f),
      mFusedGain(1.0f), mSampleRate(0), _34(false), _38(8), _3c(4) {}

/**
 * Constructs the effect.
 */
AudioFxReverbHiNin::AudioFxReverbHiNin() {
    if (sReverbHiSamplesPerFrame == 0) {
        sReverbHiSamplesPerFrame = AudioSystemNin::GetSamplesPerFrame();
    }
    initVars_();
    SetAudioFrameCount(5);
}

/**
 * Releases the work buffer and clears the delay settings and gains.
 */
void AudioFxReverbHiNin::initVars_() {
    ReleaseWorkBuffer();
    mEarlyDelaySize = 0;
    for (u32 i = 0; i < cEarlyTapCount; i++) {
        mEarlyPos[i] = 0;
        mEarlyGain[i][0] = 0.0f;
        mEarlyGain[i][1] = 0.0f;
    }
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
        mAllPassCoef[i] = 0.0f;
    }
    for (u32 i = 0; i < 2; i++) {
        mLpfInGain[i] = 0.0f;
        mLpfHistoryGain[i] = 0.0f;
        mOutAllPassDelaySize[i] = 0;
        mOutAllPassPos[i] = 0;
        mFusedGain[i] = 0.0f;
        mCrossTalk[i] = 0.0f;
    }
}

/**
 * Finalizes and destroys the effect.
 */
AudioFxReverbHiNin::~AudioFxReverbHiNin() {
    Finalize();
}

/**
 * Clears the delay lines and starts the effect.
 * @return False if the effect was already initialized.
 */
bool AudioFxReverbHiNin::Initialize() {
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
void AudioFxReverbHiNin::clearBuffer_() {
    memset(mEarlyBuffer, 0, mEarlyDelaySize * sizeof(Vector2f));
    if (mPreDelayBuffer) {
        memset(mPreDelayBuffer, 0, mPreDelaySize * sizeof(Vector2f));
    }
    for (u32 i = 0; i < cCombCount; i++) {
        memset(mCombBuffer[i], 0, mCombDelaySize[i] * sizeof(Vector2f));
    }
    for (u32 i = 0; i < cAllPassCount; i++) {
        memset(mAllPassBuffer[i], 0, mAllPassDelaySize[i] * sizeof(Vector2f));
    }
    mLpfHistory.set(0.0f, 0.0f);
    for (u32 i = 0; i < 2; i++) {
        memset(mOutAllPassBuffer[i], 0, mOutAllPassDelaySize[i] * sizeof(f32));
        mOutAllPassPos[i] = 0;
    }
    initBufferPos_();
}

/**
 * Applies the effect to one audio frame block of samples.
 * @param pSamples Sample buffer, one block per channel.
 * @param rArg Sample layout of the buffer.
 */
void AudioFxReverbHiNin::UpdateSamples(s32* pSamples, const UpdateSamplesArg& rArg) {
    if (!mIsInitialized) {
        return;
    }
    s32 frameCount = rArg.readSampleCount / (rArg.sampleCountPerAudioFrame * rArg.channelCount);
    for (s32 i = 0; i < frameCount; i++) {
        s32 sampleCount = rArg.sampleCountPerAudioFrame;
        s32* ch0 = &pSamples[sampleCount * i * rArg.channelCount];
        s32* ch1 = ch0 + sampleCount;
        updateFx_(ch0, ch1, sampleCount);
    }
}

/**
 * Applies the reverb to a stereo pair.
 * @param pCh0 Front left samples.
 * @param pCh1 Front right samples.
 * @param sampleCount Number of samples per channel.
 */
void AudioFxReverbHiNin::updateFx_(s32* pCh0, s32* pCh1, u32 sampleCount) {
    f32 earlyGain[cEarlyTapCount][2];
    memcpy(earlyGain, mEarlyGain, sizeof(earlyGain));
    f32 combCoef[cCombCount][2];
    memcpy(combCoef, mCombCoef, sizeof(combCoef));
    const f32 lpfInGain0 = mLpfInGain[0];
    const f32 lpfInGain1 = mLpfInGain[1];
    const f32 lpfHistoryGain0 = mLpfHistoryGain[0];
    const f32 lpfHistoryGain1 = mLpfHistoryGain[1];
    const f32 fusedGain0 = mFusedGain[0];
    const f32 fusedGain1 = mFusedGain[1];
    const f32 crossTalk0 = mCrossTalk[0];
    const f32 crossTalk1 = mCrossTalk[1];
    const f32 allPassCoef0 = mAllPassCoef[0];
    const f32 allPassCoef1 = mAllPassCoef[1];

    for (u32 i = 0; i < sampleCount; i++) {
        Vector2f* earlyIn = &mEarlyBuffer[mEarlyPos[2]];
        f32 earlyX = mEarlyBuffer[mEarlyPos[0]].x * earlyGain[0][0];
        f32 earlyY = mEarlyBuffer[mEarlyPos[0]].y * earlyGain[0][1];
        for (u32 j = 1; j < cEarlyTapCount; j++) {
            earlyX += mEarlyBuffer[mEarlyPos[j]].x * earlyGain[j][0];
            earlyY += mEarlyBuffer[mEarlyPos[j]].y * earlyGain[j][1];
        }
        f32 inX = *pCh0;
        f32 inY = *pCh1;
        earlyIn->set(inX, inY);

        if (mPreDelaySize != 0) {
            Vector2f* preDelay = &mPreDelayBuffer[mPreDelayPos];
            f32 delayedX = preDelay->x;
            f32 delayedY = preDelay->y;
            preDelay->set(inX, inY);
            inX = delayedX;
            inY = delayedY;
        }

        f32 combX = 0.0f;
        f32 combY = 0.0f;
        for (u32 j = 0; j < cCombCount; j++) {
            Vector2f* comb = &mCombBuffer[j][mCombPos[j]];
            f32 delayedX = comb->x;
            f32 delayedY = comb->y;
            comb->set(inX + delayedX * combCoef[j][0], inY + delayedY * combCoef[j][1]);
            combX += delayedX;
            combY += delayedY;
        }

        f32 outX = combX;
        f32 outY = combY;
        for (u32 j = 0; j < cAllPassCount; j++) {
            Vector2f* allPass = &mAllPassBuffer[j][mAllPassPos[j]];
            f32 delayedX = allPass->x;
            f32 delayedY = allPass->y;
            f32 tempX = outX + allPassCoef0 * delayedX;
            f32 tempY = outY + allPassCoef1 * delayedY;
            allPass->set(tempX, tempY);
            outX = allPassCoef0 * tempX - delayedX;
            outY = allPassCoef1 * tempY - delayedY;
        }

        f32 lpfX = lpfInGain0 * outX + lpfHistoryGain0 * mLpfHistory.x;
        f32 lpfY = lpfInGain1 * outY + lpfHistoryGain1 * mLpfHistory.y;
        mLpfHistory.set(lpfX, lpfY);

        f32* outAllPass0 = &mOutAllPassBuffer[0][mOutAllPassPos[0]];
        f32* outAllPass1 = &mOutAllPassBuffer[1][mOutAllPassPos[1]];
        f32 delayed0 = *outAllPass0;
        f32 delayed1 = *outAllPass1;
        f32 temp0 = lpfX + allPassCoef0 * delayed0;
        f32 temp1 = lpfY + allPassCoef1 * delayed1;
        *outAllPass0 = temp0;
        *outAllPass1 = temp1;
        f32 fused0 = allPassCoef0 * temp0 - delayed0;
        f32 fused1 = allPassCoef1 * temp1 - delayed1;
        mOutAllPassPos[0] = mOutAllPassPos[0] + 1 >= mOutAllPassDelaySize[0] ? 0 : mOutAllPassPos[0] + 1;
        mOutAllPassPos[1] = mOutAllPassPos[1] + 1 >= mOutAllPassDelaySize[1] ? 0 : mOutAllPassPos[1] + 1;

        f32 mix0 = earlyX + fusedGain0 * fused0;
        f32 mix1 = earlyY + fusedGain1 * fused1;
        *pCh0++ = mix0 + crossTalk1 * mix1;
        *pCh1++ = mix1 + crossTalk0 * mix0;

        for (u32 j = 0; j < cEarlyTapCount; j++) {
            mEarlyPos[j] = mEarlyPos[j] + 1 >= mEarlyDelaySize ? 0 : mEarlyPos[j] + 1;
        }
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
void AudioFxReverbHiNin::Finalize() {
    if (mIsInitialized) {
        mIsInitialized = false;
    }
}

/**
 * Applies a parameter set; delay sizes only change while no work buffer is assigned.
 * @param rParam Reverb parameters.
 * @return Always true.
 */
bool AudioFxReverbHiNin::SetParam(const AudioFxReverbHiParamNin& rParam) {
    if (!mIsBufferAssigned) {
        setupDelaySizes_(rParam);
    }
    setupGains_(rParam);
    _1a8 = rParam._38;
    _1ac = rParam._3c;
    return true;
}

/**
 * Computes the delay line lengths from the parameters and sample rate.
 * @param rParam Reverb parameters.
 */
void AudioFxReverbHiNin::setupDelaySizes_(const AudioFxReverbHiParamNin& rParam) {
    mSampleRate = rParam.mSampleRate;
    mEarlyMode = rParam.mEarlyMode;
    const u32(*earlyTable)[3] = mSampleRate == 0 ? cReverbHiEarlyDelay32k : cReverbHiEarlyDelay48k;
    mEarlyDelaySize = earlyTable[mEarlyMode][2];
    mPreDelaySize = getSampleRate_() * rParam.mPreDelayTime;
    const u32(*fusedTable)[7] = mSampleRate == 0 ? cReverbHiFusedDelay32k : cReverbHiFusedDelay48k;
    const u32* fused = fusedTable[rParam.mFusedMode];
    mCombDelaySize[0] = fused[0];
    mCombDelaySize[1] = fused[1];
    mCombDelaySize[2] = fused[2];
    mAllPassDelaySize[0] = fused[3];
    mAllPassDelaySize[1] = fused[4];
    mOutAllPassDelaySize[0] = fused[5];
    mOutAllPassDelaySize[1] = fused[6];
}

/**
 * Computes the filter coefficients and gains from the parameters.
 * @param rParam Reverb parameters.
 */
void AudioFxReverbHiNin::setupGains_(const AudioFxReverbHiParamNin& rParam) {
    f32 outGain = rParam.mOutGain * 0.6f;
    const f32* earlyGain = cReverbHiEarlyGain[mEarlyMode];
    for (u32 i = 0; i < cEarlyTapCount; i++) {
        mEarlyGain[i][0] = outGain * (earlyGain[i] * rParam.mEarlyGain);
        mEarlyGain[i][1] = mEarlyGain[i][0];
    }
    for (u32 i = 0; i < cCombCount; i++) {
        mCombCoef[i][0] = std::pow(
            10.0f, (mCombDelaySize[i] * -3.0f) / (rParam.mDecayTime * getSampleRate_()));
        mCombCoef[i][1] = mCombCoef[i][0];
    }
    mAllPassCoef[0] = rParam.mColoration;
    mAllPassCoef[1] = mAllPassCoef[0];
    f32 lpf = Mathf::clampMin(rParam.mLpfAmount, 0.05f);
    mLpfInGain[0] = lpf;
    mLpfInGain[1] = mLpfInGain[0];
    mLpfHistoryGain[0] = 1.0f - lpf;
    mLpfHistoryGain[1] = mLpfHistoryGain[0];
    mFusedGain[0] = outGain * rParam.mFusedGain;
    mFusedGain[1] = mFusedGain[0];
    mCrossTalk[0] = rParam.mCrossTalk * 0.5f;
    mCrossTalk[1] = mCrossTalk[0];
}

/**
 * Gets the work buffer size needed for the aux buffer and all delay lines.
 * @return Required work buffer size in bytes.
 */
size_t AudioFxReverbHiNin::GetRequiredMemSize() const {
    u32 earlySize = (mEarlyDelaySize * sizeof(Vector2f) + 0x1f) & ~0x1f;
    u32 preDelaySize = (mPreDelaySize * sizeof(Vector2f) + 0x1f) & ~0x1f;
    u32 combSize = 0;
    for (u32 i = 0; i < cCombCount; i++) {
        combSize += (mCombDelaySize[i] * sizeof(Vector2f) + 0x1f) & ~0x1f;
    }
    u32 allPassSize = 0;
    for (u32 i = 0; i < cAllPassCount; i++) {
        allPassSize += (mAllPassDelaySize[i] * sizeof(Vector2f) + 0x1f) & ~0x1f;
    }
    u32 outAllPassSize = 0;
    for (u32 i = 0; i < 2; i++) {
        outAllPassSize += (mOutAllPassDelaySize[i] * sizeof(f32) + 0x1f) & ~0x1f;
    }
    u32 size = AudioFxBaseNin::GetRequiredMemSize();
    size += earlySize + preDelaySize + combSize + allPassSize + outAllPassSize + 0x20;
    return size;
}

/**
 * Assigns the work buffer and splits it into the delay lines.
 * @param pBuffer Work buffer.
 * @param size Size of the work buffer.
 * @return False if a buffer is already assigned or the buffer is too small.
 */
bool AudioFxReverbHiNin::AssignWorkBuffer(void* pBuffer, u32 size) {
    if (mIsBufferAssigned) {
        return false;
    }
    AudioFxBaseNin::AssignWorkBuffer(pBuffer, size);
    uintptr_t start = reinterpret_cast<uintptr_t>(mFxWorkBuffer);
    uintptr_t current = (start + 0x1f) & ~0x1f;
    mEarlyBuffer = reinterpret_cast<Vector2f*>(current);
    current = (reinterpret_cast<uintptr_t>(mEarlyBuffer + mEarlyDelaySize) + 0x1f) & ~0x1f;
    if (mPreDelaySize != 0) {
        mPreDelayBuffer = reinterpret_cast<Vector2f*>(current);
        current = (reinterpret_cast<uintptr_t>(mPreDelayBuffer + mPreDelaySize) + 0x1f) & ~0x1f;
    }
    for (u32 i = 0; i < cCombCount; i++) {
        mCombBuffer[i] = reinterpret_cast<Vector2f*>(current);
        current = (reinterpret_cast<uintptr_t>(mCombBuffer[i] + mCombDelaySize[i]) + 0x1f) & ~0x1f;
    }
    for (u32 i = 0; i < cAllPassCount; i++) {
        mAllPassBuffer[i] = reinterpret_cast<Vector2f*>(current);
        current =
            (reinterpret_cast<uintptr_t>(mAllPassBuffer[i] + mAllPassDelaySize[i]) + 0x1f) & ~0x1f;
    }
    for (u32 i = 0; i < 2; i++) {
        mOutAllPassBuffer[i] = reinterpret_cast<f32*>(current);
        current = (reinterpret_cast<uintptr_t>(mOutAllPassBuffer[i] + mOutAllPassDelaySize[i]) + 0x1f) &
                  ~0x1f;
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
void AudioFxReverbHiNin::ReleaseWorkBuffer() {
    mIsBufferAssigned = false;
    mEarlyBuffer = nullptr;
    mPreDelayBuffer = nullptr;
    for (u32 i = 0; i < cCombCount; i++) {
        mCombBuffer[i] = nullptr;
    }
    for (u32 i = 0; i < cAllPassCount; i++) {
        mAllPassBuffer[i] = nullptr;
    }
    for (u32 i = 0; i < 2; i++) {
        mOutAllPassBuffer[i] = nullptr;
    }
    for (u32 i = 0; i < 2; i++) {
        _198[i] = nullptr;
    }
    AudioFxBaseNin::ReleaseWorkBuffer();
}

/**
 * Rewinds the delay line positions.
 */
void AudioFxReverbHiNin::initBufferPos_() {
    const u32(*earlyTable)[3] = mSampleRate == 0 ? cReverbHiEarlyDelay32k : cReverbHiEarlyDelay48k;
    for (u32 i = 0; i < cEarlyTapCount; i++) {
        mEarlyPos[i] = mEarlyDelaySize - earlyTable[mEarlyMode][i];
    }
    mPreDelayPos = 0;
    for (u32 i = 0; i < cCombCount; i++) {
        mCombPos[i] = 0;
    }
    for (u32 i = 0; i < cAllPassCount; i++) {
        mAllPassPos[i] = 0;
    }
    _1b0 = 0;
    _1b8 = 0;
}

/**
 * Gets the sample rate selected by the parameters.
 * @return Sample rate in Hz, or 0 for an unknown setting.
 */
f32 AudioFxReverbHiNin::getSampleRate_() const {
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

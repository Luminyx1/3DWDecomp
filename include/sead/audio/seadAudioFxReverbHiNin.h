#pragma once

#include "audio/seadAudioFxBaseNin.h"
#include "math/seadVector.h"

namespace sead {
class AudioFxReverbHiParamNin : public AudioFxParam {
    SEAD_RTTI_OVERRIDE(AudioFxReverbHiParamNin, AudioFxParam)

public:
    AudioFxReverbHiParamNin();
    ~AudioFxReverbHiParamNin() override {}

    f32 mPreDelayTime;
    f32 mDecayTime;
    f32 mColoration;
    f32 mLpfAmount;
    f32 mCrossTalk;
    f32 mOutGain;
    u32 mEarlyMode;
    s32 mFusedMode;
    f32 mEarlyGain;
    f32 mFusedGain;
    s32 mSampleRate;
    bool _34;
    s32 _38;
    s32 _3c;
};
static_assert(sizeof(AudioFxReverbHiParamNin) == 0x40);

class AudioFxReverbHiNin : public AudioFxBaseNin {
    SEAD_RTTI_OVERRIDE(AudioFxReverbHiNin, AudioFxBaseNin)

public:
    static const u32 cEarlyTapCount = 3;
    static const u32 cCombCount = 3;
    static const u32 cAllPassCount = 2;

    AudioFxReverbHiNin();
    ~AudioFxReverbHiNin() override;

    bool Initialize() override;
    void Finalize() override;
    void UpdateSamples(s32* pSamples, const UpdateSamplesArg& rArg) override;
    size_t GetRequiredMemSize() const override;
    bool AssignWorkBuffer(void* pBuffer, u32 size) override;
    void ReleaseWorkBuffer() override;

    bool SetParam(const AudioFxReverbHiParamNin& rParam);

private:
    void initVars_();
    void clearBuffer_();
    void updateFx_(s32* pCh0, s32* pCh1, u32 sampleCount);
    void setupDelaySizes_(const AudioFxReverbHiParamNin& rParam);
    void setupGains_(const AudioFxReverbHiParamNin& rParam);
    void initBufferPos_();
    f32 getSampleRate_() const;

    bool mIsInitialized = false;
    bool mIsBufferAssigned = false;
    s32 mSampleRate = 0;
    Vector2f* mEarlyBuffer;
    u32 mEarlyDelaySize;
    u32 mEarlyPos[cEarlyTapCount];
    f32 mEarlyGain[cEarlyTapCount][2];
    u32 mEarlyMode;
    Vector2f* mPreDelayBuffer;
    u32 mPreDelaySize;
    u32 mPreDelayPos;
    Vector2f* mCombBuffer[cCombCount];
    u32 mCombDelaySize[cCombCount];
    u32 mCombPos[cCombCount];
    f32 mCombCoef[cCombCount][2];
    Vector2f* mAllPassBuffer[cAllPassCount];
    u32 mAllPassDelaySize[cAllPassCount];
    u32 mAllPassPos[cAllPassCount];
    f32 mAllPassCoef[2];
    Vector2f mLpfHistory;
    f32 mLpfInGain[2];
    f32 mLpfHistoryGain[2];
    f32* mOutAllPassBuffer[2];
    u32 mOutAllPassDelaySize[2];
    u32 mOutAllPassPos[2];
    f32 mFusedGain[2];
    f32 mCrossTalk[2];
    void* _198[2];
    s32 _1a8 = 0;
    s32 _1ac = 0;
    u64 _1b0 = 0;
    u64 _1b8 = 0;
};
static_assert(sizeof(AudioFxReverbHiNin) == 0x1c0);
}  // namespace sead

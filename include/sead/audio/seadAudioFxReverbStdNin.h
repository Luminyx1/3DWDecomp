#pragma once

#include "audio/seadAudioFxBaseNin.h"
#include "math/seadVector.h"

namespace sead {
class AudioFxReverbStdParamNin : public AudioFxParam {
    SEAD_RTTI_OVERRIDE(AudioFxReverbStdParamNin, AudioFxParam)

public:
    enum EarlyMode {
        cEarlyMode_0,
        cEarlyMode_1,
        cEarlyMode_2,
        cEarlyMode_3,
        cEarlyMode_4,
        cEarlyMode_5,
        cEarlyMode_6,
        cEarlyMode_7
    };

    AudioFxReverbStdParamNin();
    ~AudioFxReverbStdParamNin() override {}

    f32 mPreDelayTime;
    f32 mDecayTime;
    f32 mColoration;
    f32 mLpfAmount;
    f32 mOutGain;
    EarlyMode mEarlyMode;
    s32 mFusedMode;
    f32 mEarlyGain;
    f32 mFusedGain;
    u32 mChannelCount;
    s32 mSampleRate;
    bool _34;
    s32 _38;
    s32 _3c;
};
static_assert(sizeof(AudioFxReverbStdParamNin) == 0x40);

class AudioFxReverbStdNin : public AudioFxBaseNin {
    SEAD_RTTI_OVERRIDE(AudioFxReverbStdNin, AudioFxBaseNin)

public:
    static const u32 cPairCountMax = 3;
    static const u32 cCombCount = 2;
    static const u32 cAllPassCount = 2;

    AudioFxReverbStdNin();
    ~AudioFxReverbStdNin() override;

    bool Initialize() override;
    void Finalize() override;
    void UpdateSamples(s32* pSamples, const UpdateSamplesArg& rArg) override;
    size_t GetRequiredMemSize() const override;
    bool AssignWorkBuffer(void* pBuffer, u32 size) override;
    void ReleaseWorkBuffer() override;

    bool SetParam(const AudioFxReverbStdParamNin& rParam);

private:
    void initVars_();
    void clearBuffer_();
    void updateFx2ch_(s32* pCh0, s32* pCh1, u32 sampleCount);
    void updateFx4ch_(s32* pCh0, s32* pCh1, s32* pCh2, s32* pCh3, u32 sampleCount);
    void updateFx6ch_(s32* pCh0, s32* pCh1, s32* pCh2, s32* pCh3, s32* pCh4, s32* pCh5,
                      u32 sampleCount);
    void setupDelaySizes_(const AudioFxReverbStdParamNin& rParam);
    void setupGains_(const AudioFxReverbStdParamNin& rParam);
    void initBufferPos_();
    f32 getSampleRate_() const;

    bool mIsInitialized = false;
    bool mIsBufferAssigned = false;
    bool _92 = false;
    s32 mSampleRate = 0;
    Vector2f* mEarlyBuffer[cPairCountMax];
    u32 mEarlyDelaySize;
    u32 mEarlyPos;
    f32 mEarlyCoef[2];
    f32 mEarlyGain[2];
    Vector2f* mPreDelayBuffer[cPairCountMax];
    u32 mPreDelaySize;
    u32 mPreDelayPos;
    Vector2f* mCombBuffer[cCombCount][cPairCountMax];
    u32 mCombDelaySize[cCombCount];
    u32 mCombPos[cCombCount];
    f32 mCombCoef[cCombCount][2];
    Vector2f* mAllPassBuffer[cAllPassCount][cPairCountMax];
    u32 mAllPassDelaySize[cAllPassCount];
    u32 mAllPassPos[cAllPassCount];
    f32 mAllPassCoef[2];
    Vector2f mLpfHistory[cPairCountMax];
    f32 mLpfInGain[2];
    f32 mLpfHistoryGain[2];
    f32 mOutGain[2];
    u32 mChannelCountMax = 6;
    u32 mChannelCount = 0;
    void* _1b8[6] = {};
    s32 _1e8 = 0;
    s32 _1ec = 0;
    u64 _1f0 = 0;
    u64 _1f8 = 0;
};
static_assert(sizeof(AudioFxReverbStdNin) == 0x200);
}  // namespace sead

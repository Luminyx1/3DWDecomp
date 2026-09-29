#pragma once

#include "audio/seadAudioFxBaseNin.h"
#include "math/seadVector.h"

namespace sead {
class AudioFxI3DL2ReverbParamNin : public AudioFxParam {
    SEAD_RTTI_OVERRIDE(AudioFxI3DL2ReverbParamNin, AudioFxParam)

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

    AudioFxI3DL2ReverbParamNin();
    ~AudioFxI3DL2ReverbParamNin() override {}

    s32 mRoom;
    s32 mRoomHf;
    f32 mDecayTime;
    f32 mDecayHfRatio;
    s32 mReflections;
    f32 mReflectionsDelay;
    s32 mReverb;
    f32 mReverbDelay;
    f32 mDensity;
    f32 mDiffusion;
    f32 mHfReference;
    EarlyMode mEarlyMode;
    s32 mFusedMode;
    u32 mChannelCount;
    s32 mSampleRate;
    bool _44;
    s32 _48;
    s32 _4c;
};
static_assert(sizeof(AudioFxI3DL2ReverbParamNin) == 0x50);

class AudioFxI3DL2ReverbNin : public AudioFxBaseNin {
    SEAD_RTTI_OVERRIDE(AudioFxI3DL2ReverbNin, AudioFxBaseNin)

public:
    static const u32 cPairCountMax = 3;
    static const u32 cCombCount = 2;
    static const u32 cAllPassCount = 2;

    AudioFxI3DL2ReverbNin();
    ~AudioFxI3DL2ReverbNin() override;

    bool Initialize() override;
    void Finalize() override;
    void UpdateSamples(s32* pSamples, const UpdateSamplesArg& rArg) override;
    size_t GetRequiredMemSize() const override;
    bool AssignWorkBuffer(void* pBuffer, u32 size) override;
    void ReleaseWorkBuffer() override;

    bool SetParam(const AudioFxI3DL2ReverbParamNin& rParam);

private:
    void initVars_();
    void clearBuffer_();
    void updateFx2ch_(s32* pCh0, s32* pCh1, u32 sampleCount);
    void updateFx4ch_(s32* pCh0, s32* pCh1, s32* pCh2, s32* pCh3, u32 sampleCount);
    void updateFx6ch_(s32* pCh0, s32* pCh1, s32* pCh2, s32* pCh3, s32* pCh4, s32* pCh5,
                      u32 sampleCount);
    void setupDelaySizes_(const AudioFxI3DL2ReverbParamNin& rParam);
    void setupGains_(const AudioFxI3DL2ReverbParamNin& rParam);
    void initBufferPos_();
    f32 getSampleRate_() const;
    f32 calcCombDelaySizeRate_(f32 density);
    f32 calcLpfCoefWithGain_(f32 frequency, f32 gain);
    f32 calcLpfCoef_(f32 frequency, f32 millibel);

    bool mIsInitialized = false;
    bool mIsBufferAssigned = false;
    s32 mSampleRate = 0;
    u32 mChannelCountMax = 6;
    u32 mChannelCount = 0;
    Vector2f* mReflectionsBuffer[cPairCountMax];
    u32 mReflectionsDelaySize;
    u32 mReflectionsPos;
    Vector2f* mEarlyBuffer[cPairCountMax];
    u32 mEarlyDelaySize;
    u32 mEarlyPos;
    f32 mEarlyCoef[2];
    Vector2f* mReverbDelayBuffer[cPairCountMax];
    u32 mReverbDelaySize;
    u32 mReverbDelayPos;
    Vector2f* mCombBuffer[cCombCount][cPairCountMax];
    u32 mCombDelaySize[cCombCount];
    u32 mCombPos[cCombCount];
    f32 mCombCoef[cCombCount][2];
    Vector2f mCombLpfHistory[cCombCount][cPairCountMax];
    f32 mCombLpfInGain[2];
    f32 mCombLpfHistoryGain[2];
    Vector2f* mAllPassBuffer[cAllPassCount][cPairCountMax];
    u32 mAllPassDelaySize[cAllPassCount];
    u32 mAllPassPos[cAllPassCount];
    f32 mAllPassCoef[2];
    Vector2f mLpfHistory[cPairCountMax];
    f32 mLpfInGain[2];
    f32 mLpfHistoryGain[2];
    f32 mEarlyGain[2];
    f32 mReverbGain[2];
    void* _218[6] = {};
    s32 _248 = 0;
    s32 _24c = 0;
    u64 _250 = 0;
    u64 _258 = 0;
};
static_assert(sizeof(AudioFxI3DL2ReverbNin) == 0x260);
}  // namespace sead

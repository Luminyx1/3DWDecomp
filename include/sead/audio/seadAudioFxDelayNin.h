#pragma once

#include "audio/seadAudioFxBaseNin.h"
#include "math/seadVector.h"

namespace sead {
class AudioFxDelayParamNin : public AudioFxParam {
    SEAD_RTTI_OVERRIDE(AudioFxDelayParamNin, AudioFxParam)

public:
    AudioFxDelayParamNin();
    ~AudioFxDelayParamNin() override {}

    f32 mDelayTime;
    f32 mFeedbackGain;
    f32 mOutGain;
    f32 mLpfAmount;
    u32 mChannelCount;
    s32 mSampleRate;
    bool _20;
    s32 _24;
    s32 _28;
};
static_assert(sizeof(AudioFxDelayParamNin) == 0x30);

class AudioFxDelayNin : public AudioFxBaseNin {
    SEAD_RTTI_OVERRIDE(AudioFxDelayNin, AudioFxBaseNin)

public:
    static const u32 cPairCountMax = 3;

    AudioFxDelayNin();
    ~AudioFxDelayNin() override;

    bool Initialize() override;
    void Finalize() override;
    void UpdateSamples(s32* pSamples, const UpdateSamplesArg& rArg) override;
    size_t GetRequiredMemSize() const override;
    bool AssignWorkBuffer(void* pBuffer, u32 size) override;
    void ReleaseWorkBuffer() override;

    bool SetParam(const AudioFxDelayParamNin& rParam);

private:
    void initVars_();
    void clearBuffer_();
    void updateFx2ch_(s32* pCh0, s32* pCh1, u32 sampleCount);
    void updateFx4ch_(s32* pCh0, s32* pCh1, s32* pCh2, s32* pCh3, u32 sampleCount);
    void updateFx6ch_(s32* pCh0, s32* pCh1, s32* pCh2, s32* pCh3, s32* pCh4, s32* pCh5,
                      u32 sampleCount);
    void setupDelaySizes_(const AudioFxDelayParamNin& rParam);
    void setupGains_(const AudioFxDelayParamNin& rParam);
    void initBufferPos_();
    f32 getSampleRate_() const;

    bool mIsInitialized = false;
    bool mIsBufferAssigned = false;
    s32 mSampleRate = 0;
    Vector2f* mDelayBuffer[cPairCountMax] = {};
    u32 mDelaySize;
    u32 mBufferPos;
    f32 mFeedbackGain[2];
    Vector2f mLpfHistory[cPairCountMax];
    f32 mLpfInGain[2];
    f32 mLpfHistoryGain[2];
    f32 mOutGain[2];
    u32 mChannelCountMax = 6;
    u32 mChannelCount = 0;
    void* _f8[6] = {};
    s32 _128 = 0;
    s32 _12c = 0;
    u64 _130 = 0;
    u64 _138 = 0;
};
static_assert(sizeof(AudioFxDelayNin) == 0x140);
}  // namespace sead

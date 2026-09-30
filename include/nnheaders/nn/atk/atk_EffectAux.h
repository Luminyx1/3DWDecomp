#pragma once

#include <nn/atk/atk_Global.h>
#include <nn/audio.h>
#include <nn/util/util_IntrusiveList.h>

namespace nn::atk {
class OutputMixer;

class EffectAux {
public:
    static const int ChannelCountMax = 6;

    struct UpdateSamplesArg {
        int sampleCountPerAudioFrame;
        int sampleRate;
        int audioFrameCount;
        int channelCount;
        int readSampleCount;
        OutputMode outputMode;
    };

    EffectAux();
    virtual ~EffectAux();
    virtual bool Initialize();
    virtual void Finalize();
    virtual void OnChangeOutputMode();

    bool SetChannelCount(int channelCount);
    int GetChannelCount() const;
    bool SetAudioFrameCount(int audioFrameCount);
    int GetAudioFrameCount() const;
    bool IsRemovable() const;
    bool IsEnabled() const;
    void SetEnabled(bool isEnabled);
    void SetEffectBuffer(void* effectBuffer, size_t effectBufferSize);
    void Update();
    bool AddEffect(audio::AudioRendererConfig* config, const audio::AudioRendererParameter& parameter, OutputMixer* mixer);
    void RemoveEffect(audio::AudioRendererConfig* config, OutputMixer* mixer);
    void SetEffectInputOutput(const s8* input, const s8* output, int inputCount, int outputCount);

protected:
    virtual void UpdateSamples(s32* pSamples, const UpdateSamplesArg& rArg) = 0;

public:
    util::IntrusiveListNode m_AuxLinkNode;

private:
    void* m_AuxType;
    u64 m_AudioRendererUpdateCountWhenAddedAux;
    int m_AudioFrameCount;
    int m_ChannelCount;
    bool m_IsActive;
    bool m_IsEnabled;
    void* m_EffectBuffer;
    size_t m_EffectBufferSize;
    s32* m_AuxReadBuffer;
    ChannelIndex m_ChannelSetting[ChannelCountMax];
};
static_assert(sizeof(EffectAux) == 0x68);
}  // namespace nn::atk

#pragma once
#include <nn/atk/atk_Global.h>
#include <nn/audio.h>
#include <nn/util/util_IntrusiveList.h>

namespace nn::atk {
enum SampleFormat : int;
class OutputMixer;
class EffectBase {
public:
    enum SampleRate { SampleRate_32000, SampleRate_48000 };
    enum ChannelMode { ChannelMode_Mono, ChannelMode_Stereo, ChannelMode_Quad, ChannelMode_Surround };
    EffectBase();
    virtual ~EffectBase();
    virtual size_t GetRequiredMemSize(const audio::AudioRendererParameter& parameter) const = 0;
    virtual bool AddEffect(audio::AudioRendererConfig* config, OutputMixer* mixer) = 0;
    virtual void SetEffectInputOutput(const s8* input, const s8* output, int inputCount, int outputCount) = 0;
    virtual void RemoveEffect(audio::AudioRendererConfig* config, OutputMixer* mixer) = 0;
    virtual bool IsRemovable() const = 0;
    virtual void UpdateBuffer(int channels, void** buffers, size_t bufferSize, SampleFormat format,
                              int sampleRate, OutputMode mode);
    virtual void GetChannelIndex(ChannelIndex* output, int count) const;
    virtual int GetChannelSettingCountMax() const;
    virtual void OnChangeOutputMode();
    virtual void SetEffectBuffer(void* buffer, size_t size);
    static int ConvertChannelModeToInt(ChannelMode mode);
    SampleRate GetSampleRate() const;
    bool SetSampleRate(SampleRate rate);
    util::IntrusiveListNode m_LinkNode;
private:
    bool mActive;
    SampleRate mSampleRate;
    void* mBuffer;
    size_t mBufferSize;
};
static_assert(sizeof(EffectBase) == 0x30, "Effect base size");
}

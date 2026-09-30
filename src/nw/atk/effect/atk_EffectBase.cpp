#include <nn/atk/atk_EffectBase.h>
#include <nn/util.h>

namespace nn::atk {
EffectBase::EffectBase() : mActive(false), mSampleRate(SampleRate_48000) {}
EffectBase::~EffectBase() {}
// config is the renderer configuration and mixer is the destination. The base
// implementation accepts them without installing a concrete renderer effect.
bool EffectBase::AddEffect(audio::AudioRendererConfig* config, OutputMixer* mixer) { return true; }
// input/output describe inputCount/outputCount channel indices. The base implementation does nothing.
void EffectBase::SetEffectInputOutput(const s8* input, const s8* output, int inputCount, int outputCount) {}
// config and mixer identify the installation to remove; the base implementation does nothing.
void EffectBase::RemoveEffect(audio::AudioRendererConfig* config, OutputMixer* mixer) {}
void EffectBase::OnChangeOutputMode() {}
// buffer supplies size bytes of caller-owned work memory for the effect.
void EffectBase::SetEffectBuffer(void* buffer, size_t size) {
    mBuffer = buffer;
    mBufferSize = size;
}

// mode selects mono, stereo, quad, or surround; return the corresponding channel count.
int EffectBase::ConvertChannelModeToInt(ChannelMode mode) {
    static const int channelCounts[] = {1, 2, 4, 6};

    if (static_cast<u32>(mode) >= 4) NN_UNEXPECTED_DEFAULT;
    return channelCounts[static_cast<int>(mode)];
}

// buffers contains channels sample buffers of bufferSize bytes; format and sampleRate
// describe their samples, and mode describes the output layout. No base processing is performed.
void EffectBase::UpdateBuffer(int channels, void** buffers, size_t bufferSize, SampleFormat format,
                              int sampleRate, OutputMode mode) {}
EffectBase::SampleRate EffectBase::GetSampleRate() const { return mSampleRate; }
// rate selects the effect sample rate and can only be changed while inactive.
bool EffectBase::SetSampleRate(SampleRate rate) {
    if (mActive) return false;
    mSampleRate = rate;
    return true;
}

// output has room for count channel indices; the base implementation leaves it unchanged.
void EffectBase::GetChannelIndex(ChannelIndex* output, int count) const {}
int EffectBase::GetChannelSettingCountMax() const { return 0; }
}

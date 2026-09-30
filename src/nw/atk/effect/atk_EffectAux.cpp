#include <nn/atk/atk_EffectAux.h>
#include <nn/atk/atk_HardwareManager.h>
#include <nn/atk/atk_SubMix.h>
#include <nn/util.h>

namespace nn::atk {
using detail::driver::HardwareManager;
EffectAux::EffectAux()
    : m_AudioRendererUpdateCountWhenAddedAux(0), m_AudioFrameCount(4), m_ChannelCount(4),
      m_IsActive(false), m_IsEnabled(false), m_EffectBuffer(nullptr), m_EffectBufferSize(0),
      m_AuxReadBuffer(nullptr) {
    ResetChannelIndex();
}

void EffectAux::ResetChannelIndex() {
    for (int i = 0; i < ChannelCountMax; ++i) m_ChannelSetting[i] = static_cast<ChannelIndex>(i);
}

EffectAux::~EffectAux() {}
bool EffectAux::Initialize() { return true; }
void EffectAux::Finalize() {}
void EffectAux::OnChangeOutputMode() {}
// parameter describes the renderer; reserve send, return, and CPU sample buffers.
size_t EffectAux::GetRequiredMemSize(const audio::AudioRendererParameter& parameter) const {
    return 3 * audio::GetRequiredBufferSizeForAuxSendReturnBuffer(&parameter, m_AudioFrameCount, m_ChannelCount);
}

// config and parameter describe the renderer; mixer receives this auxiliary effect.
bool EffectAux::AddEffect(audio::AudioRendererConfig* config, const audio::AudioRendererParameter& parameter,
                          OutputMixer* mixer) {
    if (m_IsActive) return false;
    size_t size = audio::GetRequiredBufferSizeForAuxSendReturnBuffer(&parameter, m_AudioFrameCount, m_ChannelCount);
    BufferSet buffers;
    SplitEffectBuffer(&buffers, m_EffectBuffer, size);
    m_AuxReadBuffer = buffers.read;
    m_AudioRendererUpdateCountWhenAddedAux.store(HardwareManager::GetInstance().GetAudioRendererUpdateCount());

    switch (mixer->GetReceiverType()) {
    case OutputReceiver::ReceiverType_SubMix:
        if (audio::AddAux(config, &m_AuxType, static_cast<SubMix*>(mixer)->GetSubMix(),
                          buffers.send, buffers.receive, size).IsFailure()) return false;
        break;
    case OutputReceiver::ReceiverType_FinalMix:
        if (audio::AddAux(config, &m_AuxType, static_cast<FinalMix*>(mixer)->GetFinalMix(),
                          buffers.send, buffers.receive, size).IsFailure()) return false;
        break;
    default: NN_UNEXPECTED_DEFAULT;
    }

    audio::SetAuxEnabled(&m_AuxType, m_IsEnabled);
    m_IsActive = true;
    return true;
}

// output receives three 64-byte-aligned slices of size bytes within caller-owned buffer.
void EffectAux::SplitEffectBuffer(BufferSet* output, void* buffer, size_t size) {
    output->send = reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(buffer) + 63) & ~uintptr_t(63));
    output->receive = reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(static_cast<char*>(output->send) + size) + 63) & ~uintptr_t(63));
    output->read = reinterpret_cast<s32*>((reinterpret_cast<uintptr_t>(static_cast<char*>(output->receive) + size) + 63) & ~uintptr_t(63));
}

// input supplies channel indices. The original ignores output, inputCount, and outputCount;
// it uses the configured channel selection for both the send and return mappings.
void EffectAux::SetEffectInputOutput(const s8* input, const s8* output, int inputCount, int outputCount) {
    s8 indices[ChannelCountMax];

    for (int i = 0; i < m_ChannelCount; ++i) indices[i] = input[m_ChannelSetting[i]];
    audio::SetAuxInputOutput(&m_AuxType, indices, indices, m_ChannelCount);
}

// config is the renderer configuration and mixer is the current effect destination.
void EffectAux::RemoveEffect(audio::AudioRendererConfig* config, OutputMixer* mixer) {
    if (!m_IsActive) return;

    switch (mixer->GetReceiverType()) {
    case OutputReceiver::ReceiverType_SubMix:
        audio::RemoveAux(config, &m_AuxType, static_cast<SubMix*>(mixer)->GetSubMix());
        break;
    case OutputReceiver::ReceiverType_FinalMix:
        audio::RemoveAux(config, &m_AuxType, static_cast<FinalMix*>(mixer)->GetFinalMix());
        break;
    default: NN_UNEXPECTED_DEFAULT;
    }

    m_IsActive = false;
}

// channelCount sets the inactive effect's channel count; channel indices reset even if active.
bool EffectAux::SetChannelCount(int channelCount) {
    ResetChannelIndex();

    if (m_IsActive) return false;
    m_ChannelCount = channelCount;
    return true;
}

// indices supplies count channel selections; changes are accepted only while inactive.
bool EffectAux::SetChannelIndex(const ChannelIndex* indices, int count) {
    if (m_IsActive) return false;
    m_ChannelCount = count;

    for (int i = 0; i < count; ++i) m_ChannelSetting[i] = indices[i];
    return true;
}

int EffectAux::GetChannelCount() const { return m_ChannelCount; }
// output receives the configured channel indices. count is ignored by the original;
// the caller must provide space for GetChannelCount() elements.
void EffectAux::GetChannelIndex(ChannelIndex* output, int count) const {
    for (int i = 0; i < m_ChannelCount; ++i) output[i] = m_ChannelSetting[i];
}

// audioFrameCount selects the number of frames buffered, while the effect is inactive.
bool EffectAux::SetAudioFrameCount(int audioFrameCount) {
    if (m_IsActive) return false;
    m_AudioFrameCount = audioFrameCount;
    return true;
}

int EffectAux::GetAudioFrameCount() const { return m_AudioFrameCount; }
bool EffectAux::IsRemovable() const {
    if (!m_IsActive) return false;

    if (m_AudioRendererUpdateCountWhenAddedAux.load() >= HardwareManager::GetInstance().GetAudioRendererUpdateCount()) return false;
    HardwareManager::GetInstance().LockAudioRenderer();
    bool result = audio::IsAuxRemovable(&m_AuxType) &&
        m_AudioRendererUpdateCountWhenAddedAux.load() < HardwareManager::GetInstance().GetAudioRendererUpdateCount();
    HardwareManager::GetInstance().UnlockAudioRenderer();
    return result;
}

bool EffectAux::IsClearable() { return IsRemovable(); }
bool EffectAux::IsEnabled() const { return m_IsEnabled; }
// isEnabled selects whether the renderer processes this effect; retain it before installation.
void EffectAux::SetEnabled(bool isEnabled) {
    m_IsEnabled = isEnabled;

    if (m_IsActive) {
        HardwareManager::GetInstance().LockAudioRenderer();
        audio::SetAuxEnabled(&m_AuxType, isEnabled);
        HardwareManager::GetInstance().UnlockAudioRenderer();
    }
}

// effectBuffer supplies effectBufferSize bytes of caller-owned work memory.
void EffectAux::SetEffectBuffer(void* effectBuffer, size_t effectBufferSize) {
    m_EffectBuffer = effectBuffer;
    m_EffectBufferSize = effectBufferSize;
}

void EffectAux::Update() {
    if (!m_IsActive) return;
    int sampleCount = audio::GetAuxSampleCount(&m_AuxType);
    UpdateSamplesArg arg;
    arg.sampleCountPerAudioFrame = sampleCount / (m_AudioFrameCount * m_ChannelCount);
    arg.sampleRate = audio::GetAuxSampleRate(&m_AuxType);
    arg.audioFrameCount = m_AudioFrameCount;
    arg.channelCount = m_ChannelCount;
    arg.outputMode = HardwareManager::GetInstance().GetOutputMode(OutputDevice_Main);
    int readCount = audio::ReadAuxSendBuffer(&m_AuxType, m_AuxReadBuffer, sampleCount);
    arg.readSampleCount = readCount;
    UpdateSamples(m_AuxReadBuffer, arg);
    audio::WriteAuxReturnBuffer(&m_AuxType, m_AuxReadBuffer, readCount);
}
}

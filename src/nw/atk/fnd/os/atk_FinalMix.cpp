#include <nn/atk/atk_FinalMix.h>
#include <nn/atk/atkfnd_WorkBufferAllocator.h>

namespace nn::audio {
void ReleaseFinalMix(AudioRendererConfig* config, FinalMixType* mix);
}
namespace nn::atk {
// effectsEnabled selects whether the single output bus needs effect storage.
size_t FinalMix::GetRequiredMemorySize(bool effectsEnabled) {
    return OutputMixer::GetRequiredMemorySize(1, effectsEnabled);
}
// config owns the audio renderer; channelCount sets the final mix width.
// effectsEnabled reserves effect resources, using bufferSize bytes supplied by buffer.
bool FinalMix::Initialize(audio::AudioRendererConfig* config, int channelCount,
                          bool effectsEnabled, void* buffer, size_t bufferSize) {
    detail::fnd::WorkBufferAllocator allocator(buffer, bufferSize);
    void* mixerBuffer = allocator.Allocate(OutputMixer::GetRequiredMemorySize(1, effectsEnabled), 8);
    OutputMixer::Initialize(1, effectsEnabled, mixerBuffer, OutputMixer::GetRequiredMemorySize(1, effectsEnabled));
    mChannelCount = channelCount;
    mReferenceCount.store(0, std::memory_order_release);
    return audio::AcquireFinalMix(config, &mFinalMix, GetBusCount() * GetChannelCount());
}
// config is the renderer configuration from which this final mix was acquired.
void FinalMix::Finalize(audio::AudioRendererConfig* config) {
    audio::ReleaseFinalMix(config, &mFinalMix);
    OutputMixer::Finalize();
}
// effect is appended to bus zero; buffer supplies bufferSize bytes of effect work memory.
bool FinalMix::AppendEffect(EffectBase* effect, void* buffer, size_t bufferSize) {
    return OutputMixer::AppendEffect(effect, 0, buffer, bufferSize);
}
// effect is appended to bus zero; buffer supplies bufferSize bytes of auxiliary work memory.
bool FinalMix::AppendEffect(EffectAux* effect, void* buffer, size_t bufferSize) {
    return OutputMixer::AppendEffect(effect, 0, buffer, bufferSize);
}
// effect identifies the effect to remove from bus zero.
bool FinalMix::RemoveEffect(EffectBase* effect) { return OutputMixer::RemoveEffect(effect, 0); }
// effect identifies the auxiliary effect to remove from bus zero.
bool FinalMix::RemoveEffect(EffectAux* effect) { return OutputMixer::RemoveEffect(effect, 0); }
bool FinalMix::ClearEffect() { return OutputMixer::ClearEffect(0); }
bool FinalMix::IsEffectEnabled() const { return mEffectsEnabled; }
OutputReceiver::ReceiverType FinalMix::GetReceiverType() const { return ReceiverType_FinalMix; }
int FinalMix::GetChannelCount() const { return mChannelCount; }
int FinalMix::GetBusCount() const { return 1; }
// count is the signed change to the number of users referencing this mix.
void FinalMix::AddReferenceCount(int count) { mReferenceCount.fetch_add(count, std::memory_order_acq_rel); }
// bus is retained for the receiver interface; this final mix always clamps sends.
bool FinalMix::IsSoundSendClampEnabled(int bus) const { return true; }
}

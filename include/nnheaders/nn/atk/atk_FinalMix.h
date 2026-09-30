#pragma once
#include <nn/audio.h>
#include <nn/os/os_Mutex.h>
#include <nn/atk/atk_EffectBase.h>
#include <nn/atk/atk_EffectAux.h>
#include <atomic>

namespace nn::atk {
class EffectBase;
class EffectAux;
class OutputReceiver {
public:
    enum ReceiverType { ReceiverType_FinalMix = 1 };
    virtual ReceiverType GetReceiverType() const = 0;
    virtual int GetChannelCount() const = 0;
    virtual int GetBusCount() const = 0;
    virtual void AddReferenceCount(int count) = 0;
    virtual bool IsSoundSendClampEnabled(int bus) const = 0;
};
class OutputMixer : public OutputReceiver {
public:
    OutputMixer();
    bool HasEffect(int bus) const;
    void UpdateEffectAux();
    void OnChangeOutputMode();
    void RemoveEffectImpl(EffectBase* effect, int bus);
    void RemoveEffectImpl(EffectAux* effect, int bus);
    void ClearEffectImpl(int bus);
    static size_t GetRequiredMemorySize(int busCount, bool effectsEnabled);
    void Initialize(int busCount, bool effectsEnabled, void* buffer, size_t bufferSize);
    void Finalize();
    bool AppendEffect(EffectBase* effect, int bus, void* buffer, size_t bufferSize);
    bool AppendEffect(EffectAux* effect, int bus, void* buffer, size_t bufferSize);
    bool RemoveEffect(EffectBase* effect, int bus);
    bool RemoveEffect(EffectAux* effect, int bus);
    bool ClearEffect(int bus);
protected:
    virtual void AppendEffectImpl(EffectBase* effect, int bus, void* buffer, size_t bufferSize);
    virtual void AppendEffectImpl(EffectAux* effect, int bus, void* buffer, size_t bufferSize);
    using EffectList = util::IntrusiveList<EffectBase, util::IntrusiveListMemberNodeTraits<EffectBase, &EffectBase::m_LinkNode>>;
    using AuxList = util::IntrusiveList<EffectAux, util::IntrusiveListMemberNodeTraits<EffectAux, &EffectAux::m_AuxLinkNode>>;
    mutable os::Mutex mMutex;
    EffectList* mEffects;
    AuxList* mAuxEffects;
    bool mEffectsEnabled;
};
class FinalMix : public OutputMixer {
public:
    static size_t GetRequiredMemorySize(bool effectsEnabled);
    bool Initialize(audio::AudioRendererConfig* config, int channelCount, bool effectsEnabled,
                    void* buffer, size_t bufferSize);
    void Finalize(audio::AudioRendererConfig* config);
    bool AppendEffect(EffectBase* effect, void* buffer, size_t bufferSize);
    bool AppendEffect(EffectAux* effect, void* buffer, size_t bufferSize);
    bool RemoveEffect(EffectBase* effect);
    bool RemoveEffect(EffectAux* effect);
    bool ClearEffect();
    bool IsEffectEnabled() const;
    ReceiverType GetReceiverType() const override;
    int GetChannelCount() const override;
    int GetBusCount() const override;
    void AddReferenceCount(int count) override;
    bool IsSoundSendClampEnabled(int bus) const override;
private:
    audio::FinalMixType mFinalMix;
    std::atomic<int> mReferenceCount;
    int mChannelCount;
};
static_assert(sizeof(OutputMixer) == 0x40, "Output mixer size");
static_assert(sizeof(FinalMix) == 0x50, "Final mix size");
}

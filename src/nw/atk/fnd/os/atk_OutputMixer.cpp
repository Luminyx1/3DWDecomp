#include <nn/atk/atk_FinalMix.h>
#include <nn/atk/atk_DriverCommand.h>
#include <nn/atk/atk_HardwareManager.h>
#include <nn/atk/atkfnd_WorkBufferAllocator.h>
#include <nn/util.h>
#include <new>

namespace nn::atk {
namespace {
class MutexLock {
public:
    // mutex is locked for this guard's lifetime.
    explicit MutexLock(os::Mutex& mutex) : mMutex(mutex) { mMutex.Lock(); }
    ~MutexLock() { mMutex.Unlock(); }
private:
    os::Mutex& mMutex;
};

struct EffectCommand : detail::Command {
    int bus;
    void* effect;
    void* buffer;
    size_t bufferSize;
    OutputMixer* mixer;
};

static_assert(sizeof(EffectCommand) == 0x40, "Effect command size");
}

OutputMixer::OutputMixer() : mMutex(true), mEffects(nullptr), mAuxEffects(nullptr), mEffectsEnabled(false) {}
// busCount is the number of effect chains; effectsEnabled controls whether they need storage.
size_t OutputMixer::GetRequiredMemorySize(int busCount, bool effectsEnabled) {
    return effectsEnabled ? size_t(busCount) * (sizeof(EffectList) + sizeof(AuxList)) : 0;
}

// busCount selects the number of chains. buffer supplies bufferSize bytes for those
// chains when effectsEnabled is true; the caller retains ownership of the storage.
void OutputMixer::Initialize(int busCount, bool effectsEnabled, void* buffer, size_t bufferSize) {
    if (effectsEnabled) {
        detail::fnd::WorkBufferAllocator allocator(buffer, bufferSize);
        mEffects = static_cast<EffectList*>(allocator.Allocate(sizeof(EffectList), alignof(EffectList), busCount));
        for (int i = 0; i < busCount; ++i) new (&mEffects[i]) EffectList;
        mAuxEffects = static_cast<AuxList*>(allocator.Allocate(sizeof(AuxList), alignof(AuxList), busCount));
        for (int i = 0; i < busCount; ++i) new (&mAuxEffects[i]) AuxList;
    }

    mEffectsEnabled = effectsEnabled;
}

void OutputMixer::Finalize() {
    mEffectsEnabled = false;
    mEffects = nullptr;
    mAuxEffects = nullptr;
}

// bus is the zero-based chain to inspect for either kind of effect.
bool OutputMixer::HasEffect(int bus) const {
    bool result = false;
    if (mEffectsEnabled) {
        MutexLock lock(mMutex);
        if (!mEffects[bus].empty() || !mAuxEffects[bus].empty()) result = true;
    }

    return result;
}

// effect is queued for bus; buffer supplies bufferSize bytes of effect work memory.
bool OutputMixer::AppendEffect(EffectBase* effect, int bus, void* buffer, size_t bufferSize) {
    if (static_cast<u32>(effect->GetSampleRate()) >= 2) NN_UNEXPECTED_DEFAULT;
    auto& driver = detail::DriverCommand::GetInstance();
    auto* command = static_cast<EffectCommand*>(driver.AllocMemory(sizeof(EffectCommand), true));
    command->type = 0x45;
    command->bus = bus;
    command->effect = effect;
    command->buffer = buffer;
    command->bufferSize = bufferSize;
    command->mixer = this;
    driver.PushCommand(command);
    AddReferenceCount(1);
    return true;
}

// effect is queued for bus; buffer supplies bufferSize bytes of auxiliary work memory.
bool OutputMixer::AppendEffect(EffectAux* effect, int bus, void* buffer, size_t bufferSize) {
    auto& driver = detail::DriverCommand::GetInstance();
    auto* command = static_cast<EffectCommand*>(driver.AllocMemory(sizeof(EffectCommand), true));
    command->type = 0x46;
    command->bus = bus;
    command->effect = effect;
    command->buffer = buffer;
    command->bufferSize = bufferSize;
    command->mixer = this;
    driver.PushCommand(command);
    AddReferenceCount(1);
    return true;
}

// effect is removed from bus; wait for the driver to finish the removal.
bool OutputMixer::RemoveEffect(EffectBase* effect, int bus) {
    auto& driver = detail::DriverCommand::GetInstance();
    auto* command = static_cast<EffectCommand*>(driver.AllocMemory(sizeof(EffectCommand), true));
    command->type = 0x47;
    command->effect = effect;
    command->bus = bus;
    command->mixer = this;
    driver.PushCommand(command);
    driver.WaitCommandReply(driver.FlushCommand(true));
    return true;
}

// effect is removed from bus; wait for the driver to finish the removal.
bool OutputMixer::RemoveEffect(EffectAux* effect, int bus) {
    auto& driver = detail::DriverCommand::GetInstance();
    auto* command = static_cast<EffectCommand*>(driver.AllocMemory(sizeof(EffectCommand), true));
    command->type = 0x48;
    command->effect = effect;
    command->bus = bus;
    command->mixer = this;
    driver.PushCommand(command);
    driver.WaitCommandReply(driver.FlushCommand(true));
    return true;
}

// bus identifies the chain to clear; wait until the driver finishes clearing it.
bool OutputMixer::ClearEffect(int bus) {
    auto& driver = detail::DriverCommand::GetInstance();
    auto* command = static_cast<EffectCommand*>(driver.AllocMemory(sizeof(EffectCommand), true));
    command->type = 0x49;
    command->bus = bus;
    command->mixer = this;
    driver.PushCommand(command);
    driver.WaitCommandReply(driver.FlushCommand(true));
    return true;
}

void OutputMixer::UpdateEffectAux() {
    mMutex.Lock();
    int count = GetBusCount();
    for (int bus = 0; bus < count; ++bus)
        for (auto it = mAuxEffects[bus].begin(); it != mAuxEffects[bus].end(); ++it) it->Update();
    mMutex.Unlock();
}

void OutputMixer::OnChangeOutputMode() {
    mMutex.Lock();
    int count = GetBusCount();
    for (int bus = 0; bus < count; ++bus)
        for (auto it = mAuxEffects[bus].begin(); it != mAuxEffects[bus].end(); ++it) it->OnChangeOutputMode();
    mMutex.Unlock();
}

// effect is installed on bus using bufferSize bytes at buffer, if its sample rate is compatible.
void OutputMixer::AppendEffectImpl(EffectBase* effect, int bus, void* buffer, size_t bufferSize) {
    auto& hardware = detail::driver::HardwareManager::GetInstance();
    if (!((hardware.GetAudioRendererParameter().sampleRate == 32000 && effect->GetSampleRate() == EffectBase::SampleRate_32000) ||
          (hardware.GetAudioRendererParameter().sampleRate == 48000 && effect->GetSampleRate() == EffectBase::SampleRate_48000))) return;
    effect->SetEffectBuffer(buffer, bufferSize);
    if (!effect->AddEffect(&detail::driver::HardwareManager::GetInstance().GetAudioRendererConfig(), this)) return;
    int count = GetChannelCount();
    s8 indices[6];
    for (int i = 0; i < count; ++i) indices[i] = bus * count + i;
    effect->SetEffectInputOutput(indices, indices, count, count);
    mMutex.Lock();
    mEffects[bus].push_back(*effect);
    mMutex.Unlock();
}

// effect is initialized and installed on bus using bufferSize bytes at buffer.
void OutputMixer::AppendEffectImpl(EffectAux* effect, int bus, void* buffer, size_t bufferSize) {
    if (!effect->Initialize()) return;
    effect->SetEffectBuffer(buffer, bufferSize);
    auto& config = detail::driver::HardwareManager::GetInstance().GetAudioRendererConfig();
    auto& parameter = detail::driver::HardwareManager::GetInstance().GetAudioRendererParameter();
    if (!effect->AddEffect(&config, parameter, this)) return;
    int count = GetChannelCount();
    s8 indices[6];
    for (int i = 0; i < count; ++i) indices[i] = bus * count + i;
    effect->SetEffectInputOutput(indices, indices, count, count);
    mMutex.Lock();
    mAuxEffects[bus].push_back(*effect);
    mMutex.Unlock();
}

// effect is removed only when found in bus's chain, then its mixer reference is released.
void OutputMixer::RemoveEffectImpl(EffectBase* effect, int bus) {
    auto& config = detail::driver::HardwareManager::GetInstance().GetAudioRendererConfig();
    MutexLock lock(mMutex);
    auto it = mEffects[bus].begin();
    for (; it != mEffects[bus].end(); ++it) {
        if (effect == &*it) break;
    }

    if (it != mEffects[bus].end()) {
        it->RemoveEffect(&config, this);
        mEffects[bus].erase(it);
        AddReferenceCount(-1);
    }
}

// effect is removed and finalized only when found in bus's auxiliary chain.
void OutputMixer::RemoveEffectImpl(EffectAux* effect, int bus) {
    auto& hardware = detail::driver::HardwareManager::GetInstance();
    mMutex.Lock();
    for (auto it = mAuxEffects[bus].begin(); it != mAuxEffects[bus].end(); ++it) {
        if (effect == &*it) {
            it->RemoveEffect(&hardware.GetAudioRendererConfig(), this);
            it->Finalize();
            mAuxEffects[bus].erase(it);
            AddReferenceCount(-1);
            break;
        }
    }

    mMutex.Unlock();
}

// bus identifies both effect chains to detach; release one mixer reference per effect.
void OutputMixer::ClearEffectImpl(int bus) {
    auto& config = detail::driver::HardwareManager::GetInstance().GetAudioRendererConfig();
    mMutex.Lock();
    int count = 0;
    for (auto it = mEffects[bus].begin(); it != mEffects[bus].end(); ++it) {
        it->RemoveEffect(&config, this);
        ++count;
    }

    mEffects[bus].clear();
    for (auto it = mAuxEffects[bus].begin(); it != mAuxEffects[bus].end(); ++it) {
        it->RemoveEffect(&config, this);
        it->Finalize();
        ++count;
    }

    mAuxEffects[bus].clear();
    AddReferenceCount(-count);
    mMutex.Unlock();
}
}

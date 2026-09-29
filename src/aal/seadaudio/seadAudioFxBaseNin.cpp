#include "audio/seadAudioFxBaseNin.h"

#include <nn/atk/atk_HardwareManager.h>
#include <nn/atk/atk_SoundSystem.h>

namespace sead {
/**
 * Constructs an effect parameter set.
 */
AudioFxParam::AudioFxParam() = default;

/**
 * Constructs an effect with no work buffer attached.
 */
AudioFxBaseNin::AudioFxBaseNin()
    : mIsMemoryPoolAttached(false), mWorkBuffer(nullptr), mWorkBufferSize(0), mFxWorkBuffer(nullptr) {}

/**
 * Detaches the work buffer memory pool and destroys the effect.
 */
AudioFxBaseNin::~AudioFxBaseNin() {
    ReleaseWorkBuffer();
}

/**
 * Gets the work buffer size required by the aux effect, rounded up to the memory pool granularity.
 * @return Required work buffer size in bytes.
 */
size_t AudioFxBaseNin::GetRequiredMemSize() const {
    u32 size = nn::atk::SoundSystem::GetRequiredEffectAuxBufferSize(this);
    return (size + 0xfff) & ~0xfff;
}

/**
 * Assigns the work buffer to the aux effect and attaches it as a memory pool.
 * @param pBuffer Work buffer.
 * @param size Size of the work buffer.
 * @return Always true.
 */
bool AudioFxBaseNin::AssignWorkBuffer(void* pBuffer, u32 size) {
    size_t auxSize = AudioFxBaseNin::GetRequiredMemSize();
    mWorkBuffer = pBuffer;
    mWorkBufferSize = auxSize;
    SetEffectBuffer(pBuffer, auxSize);
    mFxWorkBuffer = static_cast<u8*>(mWorkBuffer) + mWorkBufferSize;
    if (!mIsMemoryPoolAttached) {
        auto& hardwareManager = nn::atk::detail::driver::HardwareManager::GetInstance();
        nn::audio::AcquireMemoryPool(&hardwareManager.GetAudioRendererConfig(), &mMemoryPool, mWorkBuffer,
                                     mWorkBufferSize);
        nn::audio::RequestAttachMemoryPool(&mMemoryPool);
        while (!nn::audio::IsMemoryPoolAttached(&mMemoryPool)) {
        }
        mIsMemoryPoolAttached = true;
    }
    return true;
}

/**
 * Detaches and releases the work buffer memory pool.
 */
void AudioFxBaseNin::ReleaseWorkBuffer() {
    if (mIsMemoryPoolAttached) {
        mIsMemoryPoolAttached = false;
        auto& hardwareManager = nn::atk::detail::driver::HardwareManager::GetInstance();
        nn::audio::RequestDetachMemoryPool(&mMemoryPool);
        while (nn::audio::IsMemoryPoolAttached(&mMemoryPool)) {
        }
        nn::audio::ReleaseMemoryPool(&hardwareManager.GetAudioRendererConfig(), &mMemoryPool);
    }
}

/**
 * Initializes the effect.
 * @return Always true.
 */
bool AudioFxBaseNin::Initialize() {
    return true;
}

/**
 * Finalizes the effect.
 */
void AudioFxBaseNin::Finalize() {}
}  // namespace sead

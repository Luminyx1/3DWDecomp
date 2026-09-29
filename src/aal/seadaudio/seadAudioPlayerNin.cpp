#include "audio/seadAudioPlayerNin.h"

#include <nn/atk/atk_HardwareManager.h>
#include <nn/atk/atk_SoundSystem.h>
#include <nn/util/util_BitUtil.h>

#include "audio/seadAudioMgr.h"
#include "audio/seadAudioResetter.h"
#include "audio/seadAudioSoundDataMgrNin.h"
#include "audio/seadAudioSoundHeapNin.h"
#include "audio/seadAudioSystemNin.h"
#include "audio/seadSoundHandle.h"
#include "heap/seadHeap.h"

namespace sead {
/**
 * Constructs a handler and allocates its memory pool object.
 * @param pName Handler name.
 */
SoundMemoryPoolHandler::SoundMemoryPoolHandler(const char* pName) : mName(pName) {
    mMemoryPool = new nn::audio::MemoryPoolType();
    _30 = true;
}

/**
 * Rounds a file size up to the memory pool alignment.
 * @param fileSize File size in bytes.
 * @return Required buffer size.
 */
u32 SoundMemoryPoolHandler::calcRequireBufferSizeFromFileSize(s32 fileSize) {
    return (fileSize + 0xfff) & ~0xfff;
}

/**
 * Constructs the player with its handler array and sound data manager.
 */
AudioPlayerNin::AudioPlayerNin() {
    mMemoryPoolHandlers = new PtrArray<SoundMemoryPoolHandler>();
    mMemoryPoolHandlers->allocBuffer(100, nullptr);
    mSoundDataMgr = new AudioSoundDataMgrNin();
}

/**
 * Registers a memory pool handler if there is room.
 * @param pHandler Handler to register.
 * @return Whether the handler was registered.
 */
bool AudioPlayerNin::trySetSoundMemoryPoolHandler(SoundMemoryPoolHandler* pHandler) {
    if (mMemoryPoolHandlers->isFull()) {
        return false;
    }
    mMemoryPoolHandlers->pushBack(pHandler);
    return true;
}

/**
 * Stops all sounds and frees the player's buffers.
 */
AudioPlayerNin::~AudioPlayerNin() {
    stopAll(0);
    if (mSetupBuffer) {
        delete[] mSetupBuffer;
        mSetupBuffer = nullptr;
    }
    if (mStreamBuffer) {
        delete[] mStreamBuffer;
        mStreamBuffer = nullptr;
    }
    if (mStreamCacheBuffer) {
        delete[] mStreamCacheBuffer;
        mStreamCacheBuffer = nullptr;
    }
    if (mSoundHeap) {
        delete mSoundHeap;
        mSoundHeap = nullptr;
    }
    if (mSoundDataMgr) {
        delete mSoundDataMgr;
        mSoundDataMgr = nullptr;
    }
}

/**
 * Stops the sounds of every sound player.
 * @param fadeFrames Fade-out length in frames.
 */
void AudioPlayerNin::stopAll(s32 fadeFrames) {
    if (!isAtkEnabled_()) {
        return;
    }
    u32 count = GetSoundPlayerCount();
    for (u32 i = 0; i < count; i++) {
        GetSoundPlayer(0x4000000 + i).StopAllSound(fadeFrames);
    }
}

/**
 * Does nothing.
 */
void AudioPlayerNin::initialize() {}

/**
 * Stops all sounds and releases the heap, stream memory pool and archive.
 */
void AudioPlayerNin::finalize() {
    if (!isAtkEnabled_() || !IsAvailable()) {
        return;
    }
    stopAll(0);
    destroySoundHeap();
    if (mStreamMemoryPool) {
        nn::audio::RequestDetachMemoryPool(mStreamMemoryPool);
        while (nn::audio::IsMemoryPoolAttached(mStreamMemoryPool)) {
        }
        nn::audio::ReleaseMemoryPool(
            &nn::atk::detail::driver::HardwareManager::GetInstance().GetAudioRendererConfig(),
            mStreamMemoryPool);
    }
    shutdownDataManagement();
    mSoundDataMgr->unmountSoundArchive();
}

/**
 * Checks whether the audio system has atk enabled.
 * @return Whether atk is enabled.
 */
bool AudioPlayerNin::isAtkEnabled_() const {
    return DynamicCast<AudioSystemNin>(AudioMgr::instance()->getAudioSystem())->isAtkEnabled();
}

/**
 * Disconnects and deletes the sound heap.
 */
void AudioPlayerNin::destroySoundHeap() {
    if (!isAtkEnabled_()) {
        return;
    }
    mSoundDataMgr->connectSoundHeap(nullptr);
    if (mSoundHeap) {
        delete mSoundHeap;
        mSoundHeap = nullptr;
    }
}

/**
 * Finalizes the archive player and frees the setup and stream buffers.
 */
void AudioPlayerNin::shutdownDataManagement() {
    if (!isAtkEnabled_()) {
        return;
    }
    Finalize();
    if (mStreamBuffer) {
        delete[] mStreamBuffer;
        mStreamBuffer = nullptr;
    }
    if (mSetupBuffer) {
        delete[] mSetupBuffer;
        mSetupBuffer = nullptr;
    }
}

/**
 * Updates the archive player, under the critical section if enabled.
 */
void AudioPlayerNin::calc() {
    if (!isAtkEnabled_() || !nn::atk::SoundSystem::IsInitialized()) {
        return;
    }
    if (mIsUsingCriticalSection) {
        mCriticalSection.lock();
        Update();
        mCriticalSection.unlock();
    } else {
        Update();
    }
}

/**
 * Starts a sound by ID.
 * @param pHandle Handle to bind the sound to.
 * @param soundId Sound ID.
 * @return Whether the sound started.
 */
bool AudioPlayerNin::startSound(SoundHandle* pHandle, u32 soundId) {
    if (!isAtkEnabled_()) {
        return false;
    }
    return StartSound(pHandle, soundId).IsSuccess();
}

/**
 * Starts a sound by name.
 * @param pHandle Handle to bind the sound to.
 * @param pSoundName Sound label.
 * @return Whether the sound started.
 */
bool AudioPlayerNin::startSound(SoundHandle* pHandle, const char* pSoundName) {
    if (!isAtkEnabled_()) {
        return false;
    }
    return StartSound(pHandle, pSoundName).IsSuccess();
}

/**
 * Holds a sound by ID.
 * @param pHandle Handle to bind the sound to.
 * @param soundId Sound ID.
 * @return Whether the sound is held.
 */
bool AudioPlayerNin::holdSound(SoundHandle* pHandle, u32 soundId) {
    if (!isAtkEnabled_()) {
        return false;
    }
    return HoldSound(pHandle, soundId).IsSuccess();
}

/**
 * Holds a sound by name.
 * @param pHandle Handle to bind the sound to.
 * @param pSoundName Sound label.
 * @return Whether the sound is held.
 */
bool AudioPlayerNin::holdSound(SoundHandle* pHandle, const char* pSoundName) {
    if (!isAtkEnabled_()) {
        return false;
    }
    return HoldSound(pHandle, pSoundName).IsSuccess();
}

/**
 * Gets the number of sounds in the main archive.
 * @return Sound count.
 */
u32 AudioPlayerNin::getSoundCount() const {
    if (!isAtkEnabled_()) {
        return 0;
    }
    const nn::atk::SoundArchive* archive = mSoundDataMgr->getSoundArchive();
    if (!archive) {
        return 0;
    }
    return archive->GetSoundCount();
}

/**
 * Gets the number of sounds in the main archive and all available add-on archives.
 * @return Total sound count.
 */
u32 AudioPlayerNin::getTotalSoundCount() const {
    if (!isAtkEnabled_()) {
        return 0;
    }
    const nn::atk::SoundArchive* archive = mSoundDataMgr->getSoundArchive();
    if (!archive) {
        return 0;
    }
    u32 count = archive->GetSoundCount();
    for (u32 i = 0; i < mAddonArchiveCount - 1; i++) {
        if (GetAddonSoundArchive(i)->IsAvailable()) {
            count += GetAddonSoundArchive(i)->GetSoundCount();
        }
    }
    return count;
}

/**
 * Gets the label of a sound in the main archive.
 * @param soundId Sound ID.
 * @return Sound label, or nullptr.
 */
const char* AudioPlayerNin::getSoundName(u32 soundId) const {
    if (!isAtkEnabled_()) {
        return nullptr;
    }
    const nn::atk::SoundArchive* archive = mSoundDataMgr->getSoundArchive();
    if (!archive) {
        return nullptr;
    }
    return archive->GetItemLabel(soundId);
}

/**
 * Gets the label of a sound in the first add-on archive.
 * @param soundId Sound ID.
 * @return Sound label, or nullptr.
 */
const char* AudioPlayerNin::getAddonSoundName(u32 soundId) const {
    if (!isAtkEnabled_()) {
        return nullptr;
    }
    for (u32 i = 0; i < mAddonArchiveCount - 1; i++) {
        if (GetAddonSoundArchive(i)) {
            return GetAddonSoundArchive(i)->GetItemLabel(soundId);
        }
    }
    return nullptr;
}

/**
 * Gets the name of the first add-on archive that contains a sound.
 * @param soundId Sound ID.
 * @return Archive name, or nullptr.
 */
const char* AudioPlayerNin::getAddonArchiveName(s32 soundId) const {
    if (!isAtkEnabled_()) {
        return nullptr;
    }
    for (u32 i = 0; i < mAddonArchiveCount - 1; i++) {
        if (GetAddonSoundArchive(i) && GetAddonSoundArchive(i)->GetItemLabel(soundId)) {
            return GetAddonSoundArchiveName(i);
        }
    }
    return nullptr;
}

/**
 * Gets the ID of a sound in the main archive.
 * @param pSoundName Sound label.
 * @return Sound ID, or 0xffffffff.
 */
u32 AudioPlayerNin::getSoundId(const char* pSoundName) const {
    if (!isAtkEnabled_()) {
        return 0xffffffff;
    }
    const nn::atk::SoundArchive* archive = mSoundDataMgr->getSoundArchive();
    if (!archive) {
        return 0xffffffff;
    }
    return archive->GetItemId(pSoundName);
}

/**
 * Converts a sound ID to an add-on sound ID.
 * @param soundId Sound ID.
 * @return Add-on sound ID, or 0xffffffff.
 */
u32 AudioPlayerNin::getAddonSoundId(u32 soundId) const {
    if (!isAtkEnabled_()) {
        return 0xffffffff;
    }
    for (u32 i = 0; i < mAddonArchiveCount - 1; i++) {
        if (GetAddonSoundArchive(i)) {
            GetAddonSoundArchive(i);
            return soundId | 0x1000000;
        }
    }
    return 0xffffffff;
}

/**
 * Checks whether any add-on archive is added.
 * @return Whether an add-on archive is added.
 */
bool AudioPlayerNin::areAddonArchivesAdded() const {
    if (!isAtkEnabled_()) {
        return false;
    }
    return GetAddonSoundArchiveCount() > 0;
}

/**
 * Pauses the sounds of every sound player.
 * @param fadeFrames Fade length in frames.
 */
void AudioPlayerNin::pauseAll(s32 fadeFrames) {
    if (!isAtkEnabled_()) {
        return;
    }
    u32 count = GetSoundPlayerCount();
    if (count == 0) {
        return;
    }
    for (u32 i = 0; i < count; i++) {
        GetSoundPlayer(0x4000000 + i).PauseAllSound(true, fadeFrames);
    }
    mIsPaused = true;
}

/**
 * Pauses or unpauses the sounds of every sound player.
 * @param fadeFrames Fade length in frames.
 * @param pause Whether to pause.
 */
void AudioPlayerNin::setPauseAll_(s32 fadeFrames, bool pause) {
    if (!isAtkEnabled_()) {
        return;
    }
    u32 count = GetSoundPlayerCount();
    if (count == 0) {
        return;
    }
    for (u32 i = 0; i < count; i++) {
        GetSoundPlayer(0x4000000 + i).PauseAllSound(pause, fadeFrames);
    }
    mIsPaused = pause;
}

/**
 * Unpauses the sounds of every sound player.
 * @param fadeFrames Fade length in frames.
 */
void AudioPlayerNin::unpauseAll(s32 fadeFrames) {
    if (!isAtkEnabled_()) {
        return;
    }
    u32 count = GetSoundPlayerCount();
    if (count == 0) {
        return;
    }
    for (u32 i = 0; i < count; i++) {
        GetSoundPlayer(0x4000000 + i).PauseAllSound(false, fadeFrames);
    }
    mIsPaused = false;
}

/**
 * Sets up a sound unless starting is disabled or the audio is resetting.
 * @param pHandle Handle to bind the sound to.
 * @param soundId Sound ID.
 * @param holdFlag Whether the sound is held.
 * @param pSoundArchiveName Archive name.
 * @param pStartInfo Start parameters.
 * @return Start result.
 */
nn::atk::SoundStartable::StartResult
AudioPlayerNin::detail_SetupSound(nn::atk::SoundHandle* pHandle, u32 soundId, bool holdFlag,
                                  const char* pSoundArchiveName, const StartInfo* pStartInfo) {
    if (!isAtkEnabled_()) {
        return StartResult(StartResult::ResultCode_ErrorUser);
    }
    DynamicCast<AudioSystemNin>(AudioMgr::instance()->getAudioSystem());
    if (mIsStartDisabled || AudioMgr::instance()->getResetter()->isResetting()) {
        return StartResult(StartResult::ResultCode_ErrorUser);
    }
    return SoundArchivePlayer::detail_SetupSound(pHandle, soundId, holdFlag, pSoundArchiveName,
                                                 pStartInfo);
}

/**
 * Creates the sound heap and connects it to the sound data manager.
 * @param size Heap size.
 * @param pHeap Heap to allocate from.
 */
void AudioPlayerNin::createSoundHeap(size_t size, Heap* pHeap) {
    if (!isAtkEnabled_()) {
        return;
    }
    mSoundHeap = new (pHeap, 0x20) AudioSoundHeapNin(size, pHeap);
    mSoundDataMgr->connectSoundHeap(mSoundHeap);
}

/**
 * Initializes the archive player with a stream buffer margin.
 * @param streamBufferMargin Bytes added to the required stream buffer size.
 * @param streamReadCacheSize Stream read cache size per sound.
 * @param userParamSizePerSound User parameter size per sound.
 * @param pHeap Heap to allocate from.
 * @param addonArchiveCount Number of add-on archives.
 * @return Whether initialization succeeded.
 */
bool AudioPlayerNin::setupDataManagement(u32 streamBufferMargin, u32 streamReadCacheSize,
                                         u32 userParamSizePerSound, Heap* pHeap,
                                         u32 addonArchiveCount) {
    if (!isAtkEnabled_()) {
        return false;
    }
    const nn::atk::SoundArchive* archive = mSoundDataMgr->getSoundArchive();
    mRequiredStreamBufferSize = GetRequiredStreamBufferSize(archive);
    u32 streamBufferSize =
        mRequiredStreamBufferSize == 0 ? 0 : mRequiredStreamBufferSize + streamBufferMargin;
    return setupDataManagementInner_(*archive, streamBufferSize, streamReadCacheSize,
                                     userParamSizePerSound, pHeap, addonArchiveCount);
}

/**
 * Allocates the player buffers and initializes the archive player.
 * @param rArchive Main sound archive.
 * @param streamBufferSize Stream buffer size.
 * @param streamReadCacheSize Stream read cache size per sound.
 * @param userParamSizePerSound User parameter size per sound.
 * @param pHeap Heap to allocate from.
 * @param addonArchiveCount Number of add-on archives.
 * @return Whether initialization succeeded.
 */
bool AudioPlayerNin::setupDataManagementInner_(const nn::atk::SoundArchive& rArchive,
                                               u32 streamBufferSize, u32 streamReadCacheSize,
                                               u32 userParamSizePerSound, Heap* pHeap,
                                               u32 addonArchiveCount) {
    if (!isAtkEnabled_()) {
        return false;
    }
    mAddonArchiveCount = addonArchiveCount;
    size_t addonSize = addonArchiveCount * size_t(0x70);
    u32 setupBufferSize =
        GetRequiredMemSize(&rArchive, userParamSizePerSound) + nn::util::align_up(addonSize, 0x40);
    mSetupBuffer = new (pHeap, 0x1000) u8[setupBufferSize];

    if (streamBufferSize != 0) {
        streamBufferSize = (streamBufferSize + 0xfff) & ~0xfff;
        mStreamBuffer = new (0x1000) u8[streamBufferSize];
        mStreamMemoryPool = new nn::audio::MemoryPoolType();
        nn::audio::AcquireMemoryPool(
            &nn::atk::detail::driver::HardwareManager::GetInstance().GetAudioRendererConfig(),
            mStreamMemoryPool, mStreamBuffer, streamBufferSize);
        nn::audio::RequestAttachMemoryPool(mStreamMemoryPool);
        while (!nn::audio::IsMemoryPoolAttached(mStreamMemoryPool)) {
        }
        if (streamReadCacheSize != 0) {
            mStreamCacheBufferSize =
                GetRequiredStreamCacheSize(&rArchive, (streamReadCacheSize + 0x3f) & ~0x3f);
            mStreamCacheBuffer = new (pHeap, 0x40) u8[mStreamCacheBufferSize];
        }
    } else {
        mStreamBuffer = nullptr;
    }

    InitializeParam param;
    param.pSoundArchive = &rArchive;
    param.pSoundDataManager = mSoundDataMgr;
    param.pSetupBuffer = mSetupBuffer;
    param.setupBufferSize = setupBufferSize;
    param.pStreamBuffer = mStreamBuffer;
    param.streamBufferSize = streamBufferSize;
    param.pStreamCacheBuffer = mStreamCacheBuffer;
    param.streamCacheSize = mStreamCacheBufferSize;
    param.userParamSizePerSound = userParamSizePerSound;
    param.addonSoundArchiveCount = mAddonArchiveCount;
    bool result = Initialize(param);
    mSetupBufferSize = setupBufferSize;
    mStreamBufferSize = streamBufferSize;
    return result;
}

/**
 * Initializes the archive player with a stream buffer size rate.
 * @param rParam Setup parameters.
 * @return Whether initialization succeeded.
 */
bool AudioPlayerNin::setupDataManagement(const DataManagementSetupParam& rParam) {
    if (!isAtkEnabled_()) {
        return false;
    }
    const nn::atk::SoundArchive* archive = mSoundDataMgr->getSoundArchive();
    mRequiredStreamBufferSize = GetRequiredStreamBufferSize(archive);
    u32 streamBufferSize =
        mRequiredStreamBufferSize == 0 ?
            0 :
            static_cast<u32>(mRequiredStreamBufferSize * rParam.mStreamBufferSizeRate);
    return setupDataManagementInner_(*archive, streamBufferSize, rParam.mStreamReadCacheSize,
                                     rParam.mUserParamSizePerSound, rParam.mHeap,
                                     rParam.mAddonArchiveCount);
}

/**
 * Does nothing.
 * @param pContext Host IO context.
 */
void AudioPlayerNin::genMessage(hostio::Context* pContext) {}

/**
 * Does nothing.
 * @param pEvent Property event.
 */
void AudioPlayerNin::listenPropertyEvent(const hostio::PropertyEvent* pEvent) {}
}  // namespace sead

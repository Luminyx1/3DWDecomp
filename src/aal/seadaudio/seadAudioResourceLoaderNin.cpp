#include "audio/seadAudioResourceLoaderNin.h"

#include "audio/seadAudioMgr.h"
#include "audio/seadAudioPlayerNin.h"
#include "audio/seadAudioSoundDataMgrNin.h"
#include "audio/seadAudioSystemNin.h"
#include "heap/seadHeapMgr.h"

namespace sead {
/**
 * Constructs a resource loader without an archive.
 */
AudioResourceLoaderNin::AudioResourceLoaderNin() = default;

/**
 * Finalizes and destroys the resource loader.
 */
AudioResourceLoaderNin::~AudioResourceLoaderNin() {
    finalize();
}

/**
 * Mounts the configured sound archive.
 * @param rMgr Audio manager.
 */
void AudioResourceLoaderNin::initialize(AudioMgr& rMgr) {
    mAudioMgr = &rMgr;
    if (!DynamicCast<AudioSystemNin>(rMgr.getAudioSystem())->isAtkEnabled()) {
        return;
    }

    AudioPlayerNin* player = DynamicCast<AudioPlayerNin>(mAudioMgr->getPlayer());
    Heap* heap = mHeap ? mHeap : HeapMgr::instance()->getCurrentHeap();
    switch (mArchiveType) {
    case cArchiveType_Fs:
        if (!mFsContentRootPath.isEmpty()) {
            player->getSoundDataMgr()->setContentRootPath(mFsContentRootPath.cstr());
        }

        player->getSoundDataMgr()->mountSoundArchiveFromFs(mArchivePath, heap, mIsFileAccessInFunction,
                                                           mIsLoadLabelString);
        break;
    case cArchiveType_Memory:
        player->getSoundDataMgr()->mountSoundArchiveFromMemory(mArchiveOnMemory, heap);
        break;
    default:
        break;
    }
}

/**
 * Sets up the data management of the audio player and creates its sound heap.
 */
void AudioResourceLoaderNin::load() {
    AudioSystemNin* system = DynamicCast<AudioSystemNin>(mAudioMgr->getAudioSystem());
    if (!system->isAtkEnabled()) {
        return;
    }

    AudioPlayerNin* player = DynamicCast<AudioPlayerNin>(mAudioMgr->getPlayer());
    Heap* heap = mHeap ? mHeap : HeapMgr::instance()->getCurrentHeap();
    if (mStreamBufferSizeMergin != 0) {
        player->setupDataManagement(mStreamBufferSizeMergin, mStreamReadCacheSize, mUserParamSizePerSound, heap,
                                    system->getAddonArchiveCount());
    } else {
        AudioPlayerNin::DataManagementSetupParam param;
        param.mStreamBufferSizeRate = mStreamBufferSizeRate;
        param.mStreamReadCacheSize = mStreamReadCacheSize;
        param.mUserParamSizePerSound = mUserParamSizePerSound;
        param.mHeap = heap;
        param.mAddonArchiveCount = system->getAddonArchiveCount();
        player->setupDataManagement(param);
    }

    player->createSoundHeap(mSoundHeapSize, heap);
}

/**
 * Finalizes the resource loader (does nothing).
 */
void AudioResourceLoaderNin::finalize() {}

/**
 * Sets the heap used for the archive and the sound heap.
 * @param pHeap Heap.
 */
void AudioResourceLoaderNin::setHeap(Heap* pHeap) {
    mHeap = pHeap;
}

/**
 * Sets the stream buffer size margin.
 * @param mergin Stream buffer size margin.
 */
void AudioResourceLoaderNin::setStreamBufferSizeMergin(u32 mergin) {
    mStreamBufferSizeMergin = mergin;
}

/**
 * Sets the stream buffer size rate.
 * @param rate Stream buffer size rate.
 */
void AudioResourceLoaderNin::setStreamBufferSizeRate(f32 rate) {
    mStreamBufferSizeRate = rate;
}

/**
 * Sets the stream read cache size.
 * @param size Stream read cache size.
 */
void AudioResourceLoaderNin::setStreamReadCacheSize(u32 size) {
    mStreamReadCacheSize = size;
}

/**
 * Sets the user parameter size per sound.
 * @param size User parameter size.
 */
void AudioResourceLoaderNin::setUserParamSizePerSound(u32 size) {
    mUserParamSizePerSound = size;
}

/**
 * Sets the sound heap size.
 * @param size Sound heap size.
 */
void AudioResourceLoaderNin::setSoundHeapSize(u32 size) {
    mSoundHeapSize = size;
}

/**
 * Uses a sound archive from the file system.
 * @param rPath Archive path.
 */
void AudioResourceLoaderNin::setArchiveOnFs(const SafeString& rPath) {
    mArchivePath = rPath;
    mArchiveType = cArchiveType_Fs;
}

/**
 * Sets whether the archive file is only opened while it is accessed.
 * @param enable Whether to open the file per access.
 */
void AudioResourceLoaderNin::setFileAccessInFunction(bool enable) {
    mIsFileAccessInFunction = enable;
}

/**
 * Sets whether the label strings are loaded.
 * @param load Whether to load the label strings.
 */
void AudioResourceLoaderNin::setLoadLabelString(bool load) {
    mIsLoadLabelString = load;
}

/**
 * Sets the root path used for archives on the file system.
 * @param rPath Content root path.
 */
void AudioResourceLoaderNin::setFsContentRootPath(const SafeString& rPath) {
    mFsContentRootPath = rPath;
}

/**
 * Uses a sound archive that is already in memory.
 * @param pArchive Archive data.
 */
void AudioResourceLoaderNin::setArchiveOnMemory(const void* pArchive) {
    mArchiveOnMemory = pArchive;
    mArchiveType = cArchiveType_Memory;
}
}  // namespace sead

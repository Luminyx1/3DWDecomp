#include "audio/seadAudioSoundDataMgrNin.h"

#include "audio/seadAudioMgr.h"
#include "audio/seadAudioSoundHeapNin.h"
#include "audio/seadAudioSystemNin.h"

namespace sead {
/**
 * Constructs a sound archive that is read from the file system.
 * @param pHeap Heap for the archive header and label buffers.
 */
AudioFsSoundArchiveNin::AudioFsSoundArchiveNin(Heap* pHeap)
    : AudioSoundArchiveBaseNin(cType_Fs), mHeap(pHeap) {}

/**
 * Closes the archive and frees its buffers.
 */
AudioFsSoundArchiveNin::~AudioFsSoundArchiveNin() {
    close();

    if (mHeaderBuffer) {
        delete[] mHeaderBuffer;
        mHeaderBuffer = nullptr;
    }

    if (mLabelStringBuffer) {
        delete[] mLabelStringBuffer;
        mLabelStringBuffer = nullptr;
    }
}

/**
 * Opens the archive file below the content root and loads its header.
 * @param pFileName Archive file name (a C string).
 * @return Always true.
 */
bool AudioFsSoundArchiveNin::open(const void* pFileName) {
    if (mIsFileAccessInFunction) {
        SetFileAccessMode(FileAccessMode_InFunction);
    }

    FixedSafeString<512> path;
    path.copy(mContentRootPath);
    path.append("/");
    path.append(static_cast<const char*>(pFileName));
    Open(path.cstr());

    size_t headerSize = GetHeaderSize();
    mHeaderBuffer = new (mHeap, 0x40) u8[headerSize];
    LoadHeader(mHeaderBuffer, headerSize);

    if (mIsLoadLabelString) {
        size_t labelSize = GetLabelStringDataSize();
        mLabelStringBuffer = new (mHeap, 0x40) u8[labelSize];
        LoadLabelStringData(mLabelStringBuffer, labelSize);
    }

    return true;
}

/**
 * Closes the archive file.
 */
void AudioFsSoundArchiveNin::close() {
    if (IsAvailable()) {
        Close();
    }
}

/**
 * Sets whether the label strings are loaded with the header.
 * @param load Whether to load the label strings.
 */
void AudioFsSoundArchiveNin::setLoadLabelString(bool load) {
    mIsLoadLabelString = load;
}

/**
 * Sets the root path of the archive file.
 * @param pPath Content root path.
 */
void AudioFsSoundArchiveNin::setContentRootPath(const char* pPath) {
    mContentRootPath = pPath;
}

/**
 * Sets whether the archive file is only opened while it is accessed.
 * @param enable Whether to open the file per access.
 */
void AudioFsSoundArchiveNin::setFileAccessInFunction(bool enable) {
    mIsFileAccessInFunction = enable;
}

/**
 * Constructs a sound archive that is read from memory.
 * @param pHeap Heap (unused).
 */
AudioMemorySoundArchiveNin::AudioMemorySoundArchiveNin(Heap* pHeap)
    : AudioSoundArchiveBaseNin(cType_Memory) {}

/**
 * Closes the archive.
 */
AudioMemorySoundArchiveNin::~AudioMemorySoundArchiveNin() {
    close();
}

/**
 * Opens an archive that is already in memory.
 * @param pArchive Archive data.
 * @return Always true.
 */
bool AudioMemorySoundArchiveNin::open(const void* pArchive) {
    Initialize(pArchive);
    return true;
}

/**
 * Closes the archive.
 */
void AudioMemorySoundArchiveNin::close() {
    if (IsAvailable()) {
        Finalize();
    }
}

/**
 * Constructs a sound data manager without an archive.
 */
AudioSoundDataMgrNin::AudioSoundDataMgrNin() = default;

/**
 * Finalizes the manager and destroys the archive.
 */
AudioSoundDataMgrNin::~AudioSoundDataMgrNin() {
    if (mIsSetup) {
        Finalize();
        mIsSetup = false;
    }

    if (mSoundArchive != nullptr) {
        delete mSoundArchive;
        mSoundArchive = nullptr;
    }

    if (mWorkBuffer) {
        delete[] mWorkBuffer;
        mWorkBuffer = nullptr;
    }
}

/**
 * Sets the root path used for archives mounted from the file system.
 * @param pPath Content root path.
 */
void AudioSoundDataMgrNin::setContentRootPath(const char* pPath) {
    mContentRootPath = pPath;
}

/**
 * Sets the sound heap that data is loaded to by default.
 * @param pHeap Sound heap.
 */
void AudioSoundDataMgrNin::connectSoundHeap(AudioSoundHeapNin* pHeap) {
    if (!isNwEnabled_()) {
        return;
    }

    mDefaultSoundHeap = pHeap;

    if (pHeap == nullptr) {
        return;
    }

    const nn::atk::SoundArchive* archive = getSoundArchive();

    if (archive != nullptr) {
        mDefaultSoundHeap->setSoundDataManagement(*this, const_cast<nn::atk::SoundArchive&>(*archive));
    }
}

/**
 * Checks whether the Nintendo audio library is used.
 * @return True if the audio library is enabled.
 */
bool AudioSoundDataMgrNin::isNwEnabled_() const {
    return DynamicCast<AudioSystemNin>(AudioMgr::instance()->getAudioSystem())->mIsAtkEnabled;
}

/**
 * Gets the mounted sound archive.
 * @return Sound archive, or nullptr if none is mounted.
 */
const nn::atk::SoundArchive* AudioSoundDataMgrNin::getSoundArchive() const {
    if (!isNwEnabled_()) {
        return nullptr;
    }

    if (mSoundArchive == nullptr) {
        return nullptr;
    }

    switch (mSoundArchive->getType()) {
    case AudioSoundArchiveBaseNin::cType_Fs:
        return DynamicCast<AudioFsSoundArchiveNin>(mSoundArchive);
    case AudioSoundArchiveBaseNin::cType_Memory:
        return DynamicCast<AudioMemorySoundArchiveNin>(mSoundArchive);
    default:
        return nullptr;
    }
}

/**
 * Mounts a sound archive from the file system.
 * @param rPath Archive file name.
 * @param pHeap Heap for the archive and the manager buffers.
 * @param fileAccessInFunction Whether to only open the file while it is accessed.
 * @param loadLabelString Whether to load the label strings.
 * @return True if the manager was initialized.
 */
bool AudioSoundDataMgrNin::mountSoundArchiveFromFs(const SafeString& rPath, Heap* pHeap,
                                                   bool fileAccessInFunction, bool loadLabelString) {
    if (!isNwEnabled_()) {
        return false;
    }

    AudioFsSoundArchiveNin* archive = new (pHeap, 0x40) AudioFsSoundArchiveNin(pHeap);
    mSoundArchive = archive;
    archive->setLoadLabelString(loadLabelString);

    if (mContentRootPath != nullptr) {
        archive->setContentRootPath(mContentRootPath);
    }

    archive->setFileAccessInFunction(fileAccessInFunction);
    archive->open(rPath.cstr());
    return setupManager_(pHeap);
}

/**
 * Initializes the Nintendo sound data manager for the mounted archive.
 * @param pHeap Heap for the manager buffer.
 * @return True if the manager was initialized.
 */
bool AudioSoundDataMgrNin::setupManager_(Heap* pHeap) {
    if (!isNwEnabled_()) {
        return false;
    }

    u32 size = GetRequiredMemSize(getSoundArchive());
    mWorkBuffer = new (pHeap, 0x40) u8[size];
    bool result = Initialize(getSoundArchive(), mWorkBuffer, size);
    mIsSetup = true;
    return result;
}

/**
 * Mounts a sound archive that is already in memory.
 * @param pArchive Archive data.
 * @param pHeap Heap for the archive and the manager buffer.
 * @return True if the manager was initialized.
 */
bool AudioSoundDataMgrNin::mountSoundArchiveFromMemory(const void* pArchive, Heap* pHeap) {
    if (!isNwEnabled_()) {
        return false;
    }

    mSoundArchive = new (pHeap, 0x40) AudioMemorySoundArchiveNin(pHeap);
    mSoundArchive->open(pArchive);
    return setupManager_(pHeap);
}

/**
 * Finalizes the manager and destroys the mounted archive.
 */
void AudioSoundDataMgrNin::unmountSoundArchive() {
    if (!isNwEnabled_()) {
        return;
    }

    if (mIsSetup) {
        Finalize();
        mIsSetup = false;
    }

    if (mSoundArchive != nullptr) {
        delete mSoundArchive;
    }

    mSoundArchive = nullptr;

    if (mWorkBuffer) {
        delete[] mWorkBuffer;
        mWorkBuffer = nullptr;
    }
}

/**
 * Loads sound data to a sound heap.
 * @param itemId Item ID.
 * @param loadFlag Kinds of data to load.
 * @param loadBlockSize Size of the load blocks.
 * @param pHeap Sound heap, or nullptr for the default one.
 * @return True if the data was loaded.
 */
bool AudioSoundDataMgrNin::loadData(u32 itemId, u32 loadFlag, u32 loadBlockSize, AudioSoundHeapNin* pHeap) {
    if (!isNwEnabled_()) {
        return false;
    }

    if (!tryGetDefaultSoundHeapAndCheckReady_(&pHeap)) {
        return false;
    }

    return LoadData(itemId, pHeap, loadFlag, loadBlockSize);
}

/**
 * Picks the default sound heap if none is given and checks that data can be loaded.
 * @param ppHeap Sound heap, replaced by the default one if it is nullptr.
 * @return True if data can be loaded.
 */
bool AudioSoundDataMgrNin::tryGetDefaultSoundHeapAndCheckReady_(AudioSoundHeapNin** ppHeap) const {
    if (!isNwEnabled_()) {
        return false;
    }

    if (*ppHeap == nullptr) {
        *ppHeap = mDefaultSoundHeap;
        if (*ppHeap == nullptr) {
            return false;
        }
    }

    return IsAvailable();
}

/**
 * Loads sound data to a sound heap.
 * @param pItemName Item label.
 * @param loadFlag Kinds of data to load.
 * @param loadBlockSize Size of the load blocks.
 * @param pHeap Sound heap, or nullptr for the default one.
 * @return True if the data was loaded.
 */
bool AudioSoundDataMgrNin::loadData(const char* pItemName, u32 loadFlag, u32 loadBlockSize,
                                    AudioSoundHeapNin* pHeap) {
    if (!isNwEnabled_()) {
        return false;
    }

    if (!tryGetDefaultSoundHeapAndCheckReady_(&pHeap)) {
        return false;
    }

    return LoadData(pItemName, pHeap, loadFlag, loadBlockSize);
}
}  // namespace sead

#include "Project/File/FileLoader.hpp"

#include <filedevice/seadFileDeviceMgr.h>
#include <heap/seadHeapMgr.h>
#include <resource/seadArchiveRes.h>
#include <resource/seadResourceMgr.h>
#include <thread/seadDelegateThread.h>
#include <thread/seadThread.h>

#include "Library/File/FileUtil.hpp"
#include "Library/File/Holder/ArchiveHolder.hpp"
#include "Library/File/Holder/FileLoaderThread.hpp"
#include "Library/File/Holder/SoundItemEntry.hpp"
#include "Library/File/Holder/SoundItemHolder.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/File/ArchiveEntry.hpp"

namespace al {
static sead::SafeString sDeviceName = "main";

/**
 * Creates the loader thread and entry holders and finds the main file device.
 * @param threadPriority Loader thread priority.
 * @param unk Unused.
 */
FileLoader::FileLoader(s32 threadPriority, bool unk) {
    mLoaderThread = new FileLoaderThread(threadPriority);
    mArchiveHolder = new ArchiveHolder();
    mSoundItemHolder = new SoundItemHolder();
    mFileDevice = sead::FileDeviceMgr::instance()->findDevice(sDeviceName);
}

/**
 * Checks whether a file exists.
 * @param rPath File path.
 * @param pDevice File device, or nullptr for the main device.
 * @return True if the file exists.
 */
bool FileLoader::isExistFile(const sead::SafeString& rPath, sead::FileDevice* pDevice) const {
    sead::FileDevice* device = getFileDevice(rPath, pDevice);
    bool isExist = false;
    device->tryIsExistFile(&isExist, rPath);
    return isExist;
}

/**
 * Gets the file device to use for a path.
 * @param rPath File path.
 * @param pDevice File device, or nullptr for the main device.
 * @return The file device.
 */
sead::FileDevice* FileLoader::getFileDevice(const sead::SafeString& rPath,
                                            sead::FileDevice* pDevice) const {
    return pDevice ?: mFileDevice;
}

/**
 * Checks whether an archive exists.
 * @param rPath Archive path.
 * @param pDevice File device, or nullptr for the main device.
 * @return True if the archive exists.
 */
bool FileLoader::isExistArchive(const sead::SafeString& rPath, sead::FileDevice* pDevice) const {
    return isExistFile(rPath, pDevice);
}

/**
 * Gets the size of a file.
 * @param rPath File path.
 * @param pDevice File device, or nullptr for the main device.
 * @return The file size.
 */
u32 FileLoader::getFileSize(const sead::SafeString& rPath, sead::FileDevice* pDevice) const {
    sead::FileDevice* device = getFileDevice(rPath, pDevice);
    u32 size = 0;
    device->tryGetFileSize(&size, rPath);
    return size;
}

/**
 * Loads a file into a newly allocated buffer.
 * @param rPath File path.
 * @param alignment Buffer alignment.
 * @param pDevice File device, or nullptr for the main device.
 * @return The loaded data.
 */
u8* FileLoader::loadFile(const sead::SafeString& rPath, s32 alignment, sead::FileDevice* pDevice) {
    sead::FileDevice::LoadArg loadArg;
    loadArg.alignment = alignment;
    loadArg.path = rPath;
    return getFileDevice(rPath, pDevice)->tryLoad(loadArg);
}

/**
 * Loads an archive, synchronously on the main thread or through the loader thread otherwise.
 * @param rPath Archive path.
 * @param pDevice File device, or nullptr for the main device.
 * @return The archive.
 */
sead::ArchiveRes* FileLoader::loadArchive(const sead::SafeString& rPath,
                                          sead::FileDevice* pDevice) {
    ArchiveEntry* entry = mArchiveHolder->tryFindEntry(rPath);
    if (entry) {
        if (entry->mFileState != FileState::IsLoadDone) {
            entry->waitLoadDone();
        }
    } else if (sead::ThreadMgr::instance()->isMainThread()) {
        sead::ResourceMgr::LoadArg loadArg;
        loadArg.device = getFileDevice(rPath, pDevice);
        loadArg.path = rPath;
        loadArg.load_data_alignment = calcFileAlignment(rPath);
        sead::Resource* resource =
            sead::ResourceMgr::instance()->tryLoad(loadArg, "sarc", nullptr);
        return sead::DynamicCast<sead::ArchiveRes>(resource);
    } else {
        sead::Heap* heap = sead::HeapMgr::instance()->getCurrentHeap();
        entry = requestLoadArchive(rPath, heap, getFileDevice(rPath, pDevice));
        entry->waitLoadDone();
    }

    return entry->getArchiveRes();
}

/**
 * Requests an asynchronous archive load.
 * @param rPath Archive path.
 * @param pHeap Heap to load into.
 * @param pDevice File device, or nullptr for the main device.
 * @return The new archive entry.
 */
ArchiveEntry* FileLoader::requestLoadArchive(const sead::SafeString& rPath, sead::Heap* pHeap,
                                             sead::FileDevice* pDevice) {
    ArchiveEntry* entry =
        mArchiveHolder->addNewLoadRequestEntry(rPath, pHeap, getFileDevice(rPath, pDevice));
    mLoaderThread->requestLoadFile(entry);
    return entry;
}

/**
 * Requests an asynchronous archive load unless it was already requested.
 * @param rPath Archive path.
 * @param pHeap Heap to load into.
 * @param pDevice File device, or nullptr for the main device.
 * @return True if a new request was made.
 */
bool FileLoader::tryRequestLoadArchive(const sead::SafeString& rPath, sead::Heap* pHeap,
                                       sead::FileDevice* pDevice) {
    if (mArchiveHolder->tryFindEntry(rPath)) {
        return false;
    }

    requestLoadArchive(rPath, pHeap, pDevice);
    return true;
}

/**
 * Loads a sound item, waiting until it is done.
 * @param itemId Sound item id.
 * @param unk Unknown.
 * @param pLoader Audio resource loader.
 * @return True if the load succeeded.
 */
bool FileLoader::loadSoundItem(u32 itemId, u32 unk, IAudioResourceLoader* pLoader) {
    SoundItemEntry* entry = mSoundItemHolder->tryFindEntry(itemId, pLoader);
    if (!entry) {
        entry = requestLoadSoundItem(itemId, unk, pLoader);
        entry->waitLoadDone();
    } else if (entry->mFileState != FileState::IsLoadDone) {
        entry->waitLoadDone();
    }

    return entry->isLoadSuccess();
}

/**
 * Requests an asynchronous sound item load.
 * @param itemId Sound item id.
 * @param unk Unknown.
 * @param pLoader Audio resource loader.
 * @return The new sound item entry.
 */
SoundItemEntry* FileLoader::requestLoadSoundItem(u32 itemId, u32 unk,
                                                 IAudioResourceLoader* pLoader) {
    SoundItemEntry* entry = mSoundItemHolder->addNewLoadRequestEntry(itemId, unk, pLoader);
    mLoaderThread->requestLoadFile(entry);
    return entry;
}

/**
 * Requests an asynchronous sound item load unless it was already requested.
 * @param itemId Sound item id.
 * @param pLoader Audio resource loader.
 * @return True if a new request was made.
 */
bool FileLoader::tryRequestLoadSoundItem(u32 itemId, IAudioResourceLoader* pLoader) {
    if (mSoundItemHolder->tryFindEntry(itemId, pLoader)) {
        return false;
    }

    requestLoadSoundItem(itemId, -1, pLoader);
    return true;
}

/**
 * Requests loads for every existing archive in a preload list, once.
 * @param rPreLoadList Byaml preload list.
 * @param pHeap Heap to load into.
 * @param pLoader Audio resource loader.
 */
void FileLoader::requestPreLoadFile(const ByamlIter& rPreLoadList, sead::Heap* pHeap,
                                    IAudioResourceLoader* pLoader) {
    if (mIsFilePreloaded) {
        return;
    }

    for (s32 i = 0; i < rPreLoadList.getSize(); i++) {
        ByamlIter iter;
        rPreLoadList.tryGetIterByIndex(&iter, i);

        const char* type;
        iter.tryGetStringByKey(&type, "Type");

        if (!isEqualString(type, "SoundItem") && isEqualString(type, "Archive")) {
            const char* path;
            iter.tryGetStringByKey(&path, "Path");
            if (isExistArchive(path, nullptr)) {
                tryRequestLoadArchive(path, pHeap, nullptr);
            }
        }
    }

    mIsFilePreloaded = true;
}

/**
 * Waits until every requested archive is loaded.
 */
void FileLoader::waitLoadDoneAllFile() {
    mArchiveHolder->waitLoadDoneAll();
}

/**
 * Clears all archive and sound item entries.
 */
void FileLoader::clearAllEntry() {
    mArchiveHolder->clearEntry();
    mSoundItemHolder->clearEntry();
    mIsFilePreloaded = false;
}

/**
 * Sets the loader thread priority.
 * @param priority Thread priority.
 */
void FileLoader::setThreadPriority(s32 priority) {
    mLoaderThread->getThread()->setPriority(priority);
}
}  // namespace al

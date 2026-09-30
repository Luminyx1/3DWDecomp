#include "Project/File/ArchiveEntry.hpp"

#include <resource/seadArchiveRes.h>

#include "Library/File/FileUtil.hpp"

namespace al {
/**
 * Constructs an empty archive entry.
 */
ArchiveEntry::ArchiveEntry() = default;

/**
 * Loads the archive and signals completion.
 */
void ArchiveEntry::load() {
    sead::Resource* resource = sead::ResourceMgr::instance()->tryLoad(mLoadArg, "sarc", nullptr);
    mArchiveRes = sead::DynamicCast<sead::ArchiveRes>(resource);
    sendMessageDone();
}

/**
 * Sets up the load arguments for the archive.
 * @param rFileName Archive path.
 * @param pHeap Heap to load into.
 * @param pDevice File device.
 */
void ArchiveEntry::setLoadRequestInfo(const sead::SafeString& rFileName, sead::Heap* pHeap,
                                      sead::FileDevice* pDevice) {
    setFileName(rFileName);
    mLoadArg.path = getFileName();
    mLoadArg.device = pDevice;
    mLoadArg.instance_heap = pHeap;
    mLoadArg.load_data_heap = pHeap;
    mLoadArg.load_data_alignment = calcFileAlignment(rFileName);
    setLoadStateRequested();
}

/**
 * Gets the loaded archive.
 * @return The archive.
 */
sead::ArchiveRes* ArchiveEntry::getArchiveRes() {
    return mArchiveRes;
}

/**
 * Clears the entry.
 */
void ArchiveEntry::clear() {
    FileEntryBase::clear();
    mArchiveRes = nullptr;
}
}  // namespace al

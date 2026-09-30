#include "Library/File/Holder/ArchiveHolder.hpp"

#include <basis/seadNew.h>

#include "Project/Base/StringUtil.hpp"
#include "Project/File/ArchiveEntry.hpp"

namespace al {
/**
 * Allocates the archive entries.
 */
ArchiveHolder::ArchiveHolder() {
    mArchiveEntries.allocBufferAssert(0x400, nullptr);
}

/**
 * Takes the next free entry and sets up its load request.
 * @param rFileName Archive path.
 * @param pHeap Heap to load into.
 * @param pDevice File device.
 * @return The entry.
 */
ArchiveEntry* ArchiveHolder::addNewLoadRequestEntry(const sead::SafeString& rFileName,
                                                    sead::Heap* pHeap, sead::FileDevice* pDevice) {
    ArchiveEntry* entry = mArchiveEntries.get(mSize);
    entry->setLoadRequestInfo(rFileName, pHeap, pDevice);
    mSize++;
    return entry;
}

/**
 * Finds the entry of an archive.
 * @param rFileName Archive path.
 * @return The entry or nullptr.
 */
ArchiveEntry* ArchiveHolder::tryFindEntry(const sead::SafeString& rFileName) {
    for (s32 i = 0; i < mSize; i++) {
        ArchiveEntry* entry = mArchiveEntries.get(i);
        if (isEqualString(entry->getFileName(), rFileName)) {
            return entry;
        }
    }
    return nullptr;
}

/**
 * Waits until every entry finished loading.
 */
void ArchiveHolder::waitLoadDoneAll() {
    for (s32 i = 0; i < mSize; i++) {
        ArchiveEntry* entry = mArchiveEntries.get(i);
        if (entry->mFileState != FileState::IsLoadDone) {
            entry->waitLoadDone();
        }
    }
}

/**
 * Clears every entry.
 */
void ArchiveHolder::clearEntry() {
    for (s32 i = 0; i < mSize; i++) {
        mArchiveEntries.get(i)->clear();
    }
    mSize = 0;
}
}  // namespace al

#include "Library/File/Holder/ArchiveHolder.hpp"

#include "Library/File/Holder/ArchiveEntry.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * @brief Constructs a holder with room for 1024 archive entries.
 */
ArchiveHolder::ArchiveHolder() {
    mEntries.allocBufferAssert(1024, nullptr);
}

/**
 * @brief Takes the next free entry and sets it up for a load request.
 * @param rFileName The path of the archive to load.
 * @param pHeap The heap the archive is loaded into.
 * @param pFileDevice The file device the archive is read from.
 * @return The entry that was set up.
 */
ArchiveEntry* ArchiveHolder::addNewLoadRequestEntry(const sead::SafeString& rFileName,
                                                    sead::Heap* pHeap,
                                                    sead::FileDevice* pFileDevice) {
    ArchiveEntry* pEntry = mEntries.get(mNumEntries);
    pEntry->setLoadRequestInfo(rFileName, pHeap, pFileDevice);
    mNumEntries++;
    return pEntry;
}

/**
 * @brief Finds the entry for an archive by its path.
 * @param rFileName The path of the archive.
 * @return The matching entry, or nullptr if there is none.
 */
ArchiveEntry* ArchiveHolder::tryFindEntry(const sead::SafeString& rFileName) {
    for (s32 i = 0; i < mNumEntries; i++) {
        ArchiveEntry* pEntry = mEntries.get(i);
        if (isEqualString(pEntry->getFileName(), rFileName)) {
            return pEntry;
        }
    }

    return nullptr;
}

/**
 * @brief Blocks until every requested entry has finished loading.
 */
void ArchiveHolder::waitLoadDoneAll() {
    for (s32 i = 0; i < mNumEntries; i++) {
        ArchiveEntry* pEntry = mEntries.get(i);
        if (pEntry->mFileState != 3) {
            pEntry->waitLoadDone();
        }
    }
}

/**
 * @brief Clears every entry and empties the holder.
 */
void ArchiveHolder::clearEntry() {
    for (s32 i = 0; i < mNumEntries; i++) {
        mEntries.get(i)->clear();
    }

    mNumEntries = 0;
}
}  // namespace al

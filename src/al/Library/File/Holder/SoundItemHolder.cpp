#include "Library/File/Holder/SoundItemHolder.hpp"

#include "Library/File/Holder/SoundItemEntry.hpp"

namespace al {
/**
 * @brief Constructs a holder with room for 640 sound item entries.
 */
SoundItemHolder::SoundItemHolder() {
    mEntries.allocBufferAssert(640, nullptr);
}

/**
 * @brief Takes the next free entry and sets it up for a load request.
 * @param itemId The id of the sound item to load.
 * @param param The secondary parameter passed to the resource loader.
 * @param pResourceLoader The loader used to load the sound item.
 * @return The entry that was set up.
 */
SoundItemEntry* SoundItemHolder::addNewLoadRequestEntry(u32 itemId, u32 param,
                                                        IAudioResourceLoader* pResourceLoader) {
    SoundItemEntry* pEntry = mEntries.get(mNumEntries);
    pEntry->setLoadRequestInfo(itemId, param, pResourceLoader);
    mNumEntries++;
    return pEntry;
}

/**
 * @brief Finds the entry for a sound item loaded by a given loader.
 * @param itemId The id of the sound item.
 * @param pResourceLoader The loader the item was requested with.
 * @return The matching entry, or nullptr if there is none.
 */
SoundItemEntry* SoundItemHolder::tryFindEntry(u32 itemId, IAudioResourceLoader* pResourceLoader) {
    for (s32 i = 0; i < mNumEntries; i++) {
        SoundItemEntry* pEntry = mEntries.get(i);
        if (pEntry->getSoundItemId() == itemId && pEntry->mResourceLoader == pResourceLoader) {
            return pEntry;
        }
    }

    return nullptr;
}

/**
 * @brief Blocks until every requested entry has finished loading.
 */
void SoundItemHolder::waitLoadDoneAll() {
    for (s32 i = 0; i < mNumEntries; i++) {
        SoundItemEntry* pEntry = mEntries.get(i);
        if (pEntry->mFileState != 3) {
            pEntry->waitLoadDone();
        }
    }
}

/**
 * @brief Clears every entry and empties the holder.
 */
void SoundItemHolder::clearEntry() {
    for (s32 i = 0; i < mNumEntries; i++) {
        mEntries.get(i)->clear();
    }

    mNumEntries = 0;
}
}  // namespace al

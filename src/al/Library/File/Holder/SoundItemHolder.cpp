#include "Library/File/Holder/SoundItemHolder.hpp"

#include "Library/File/Holder/SoundItemEntry.hpp"

namespace al {
/**
 * Allocates the sound item entries.
 */
SoundItemHolder::SoundItemHolder() {
    mSoundItemEntries.allocBufferAssert(640, nullptr);
}

/**
 * Takes the next free entry and sets up its load request.
 * @param itemId Sound item id.
 * @param unk Unknown.
 * @param pLoader Audio resource loader.
 * @return The entry.
 */
SoundItemEntry* SoundItemHolder::addNewLoadRequestEntry(u32 itemId, u32 unk,
                                                        IAudioResourceLoader* pLoader) {
    SoundItemEntry* entry = mSoundItemEntries.get(mSize);
    entry->setLoadRequestInfo(itemId, unk, pLoader);
    mSize++;
    return entry;
}

/**
 * Finds the entry of a sound item.
 * @param itemId Sound item id.
 * @param pLoader Audio resource loader.
 * @return The entry or nullptr.
 */
SoundItemEntry* SoundItemHolder::tryFindEntry(u32 itemId, IAudioResourceLoader* pLoader) {
    for (s32 i = 0; i < mSize; i++) {
        SoundItemEntry* entry = mSoundItemEntries.get(i);
        if (entry->getSoundItemId() == itemId && entry->getAudioResourceLoader() == pLoader) {
            return entry;
        }
    }

    return nullptr;
}

/**
 * Waits until every entry finished loading.
 */
void SoundItemHolder::waitLoadDoneAll() {
    for (s32 i = 0; i < mSize; i++) {
        SoundItemEntry* entry = mSoundItemEntries.get(i);
        if (entry->mFileState != FileState::IsLoadDone) {
            entry->waitLoadDone();
        }
    }
}

/**
 * Clears every entry.
 */
void SoundItemHolder::clearEntry() {
    for (s32 i = 0; i < mSize; i++) {
        mSoundItemEntries.get(i)->clear();
    }

    mSize = 0;
}
}  // namespace al

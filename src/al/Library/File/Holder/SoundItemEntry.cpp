#include "Library/File/Holder/SoundItemEntry.hpp"

namespace al {
/**
 * Constructs an empty sound item entry.
 */
SoundItemEntry::SoundItemEntry() = default;

/**
 * Loads the sound item and signals completion.
 */
void SoundItemEntry::load() {
    mIsLoadSuccess = mResourceLoader->tryLoad(mItemId, _bc);
    sendMessageDone();
}

/**
 * Sets up the sound item load request.
 * @param itemId Sound item id.
 * @param unk Unknown.
 * @param pLoader Audio resource loader.
 */
void SoundItemEntry::setLoadRequestInfo(u32 itemId, u32 unk, IAudioResourceLoader* pLoader) {
    mItemId = itemId;
    _bc = unk;
    mResourceLoader = pLoader;
    setLoadStateRequested();
}

/**
 * Checks whether the load succeeded.
 * @return True on success.
 */
bool SoundItemEntry::isLoadSuccess() const {
    return mIsLoadSuccess;
}

/**
 * Gets the sound item id.
 * @return The id.
 */
u32 SoundItemEntry::getSoundItemId() const {
    return mItemId;
}

/**
 * Clears the entry.
 */
void SoundItemEntry::clear() {
    FileEntryBase::clear();
    mItemId = AudioConst::SOUND_ID_INVALID;
    _bc = -1;
    mResourceLoader = nullptr;
    mIsLoadSuccess = false;
}
}  // namespace al

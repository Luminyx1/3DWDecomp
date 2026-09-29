#include "Library/File/Holder/SoundItemEntry.hpp"

namespace al {
/**
 * @brief Constructs an empty sound item entry.
 */
SoundItemEntry::SoundItemEntry() = default;

/**
 * @brief Loads the requested sound item through its resource loader and signals completion.
 */
void SoundItemEntry::load() {
    mIsLoadSuccess = mResourceLoader->tryLoad(mItemId, _BC);
    sendMessageDone();
}

/**
 * @brief Sets up the entry for a load request and marks it as requested.
 * @param itemId The id of the sound item to load.
 * @param param The secondary parameter passed to the resource loader.
 * @param pResourceLoader The loader used to load the sound item.
 */
void SoundItemEntry::setLoadRequestInfo(u32 itemId, u32 param,
                                        IAudioResourceLoader* pResourceLoader) {
    mItemId = itemId;
    _BC = param;
    mResourceLoader = pResourceLoader;
    setLoadStateRequested();
}

/**
 * @brief Checks whether the last load succeeded.
 * @return True if the sound item was loaded successfully.
 */
bool SoundItemEntry::isLoadSuccess() const {
    return mIsLoadSuccess;
}

/**
 * @brief Gets the id of the sound item held by this entry.
 * @return The sound item id.
 */
u32 SoundItemEntry::getSoundItemId() const {
    return mItemId;
}

/**
 * @brief Resets the entry to its empty state.
 */
void SoundItemEntry::clear() {
    FileEntryBase::clear();
    mItemId = AudioConst::SOUND_ID_INVALID;
    _BC = -1;
    mResourceLoader = nullptr;
    mIsLoadSuccess = false;
}
}  // namespace al

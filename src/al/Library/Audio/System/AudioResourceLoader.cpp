#include "Project/Audio/System/AudioResourceLoader.hpp"

namespace al {
bool loadSoundItem(u32 id, u32 loadFlag, IAudioResourceLoader* pLoader);

/**
 * Constructs an empty layer.
 * @param pLoader Loader used to load sound items.
 * @param itemNum Number of sound items the layer can track.
 */
AudioResourceLayer::AudioResourceLayer(IAudioResourceLoader* pLoader, s32 itemNum) {
    mLoader = pLoader;
    mLoadedItemIds.allocBuffer(itemNum, nullptr);
}

/**
 * Sets the layer name and forgets all loaded items.
 * @param rName Layer name.
 */
void AudioResourceLayer::setData(const sead::SafeString& rName) {
    mName = rName;
    mLoadedItemIds.clear();
}

/**
 * Records a loaded sound item.
 * @param id Item id.
 */
void AudioResourceLayer::addLoadedSoundItemId(u32 id) {
    mLoadedItemIds.pushBack(id);
}

/**
 * Loads a sound item and records it.
 * @param id Item id.
 * @param loadFlag Load flags.
 * @return True on success.
 */
bool AudioResourceLayer::loadSoundItem(u32 id, u32 loadFlag) {
    bool isLoaded = al::loadSoundItem(id, loadFlag, mLoader);
    if (isLoaded) {
        mLoadedItemIds.pushBack(id);
    }

    return isLoaded;
}

/**
 * Checks whether this layer has loaded a sound item.
 * @param id Item id.
 * @return True if loaded.
 */
bool AudioResourceLayer::isLoadedSoundItem(u32 id) {
    for (s32 i = 0; i < mLoadedItemIds.size(); i++) {
        if (mLoadedItemIds(i) == id) {
            return true;
        }
    }

    return false;
}
}  // namespace al

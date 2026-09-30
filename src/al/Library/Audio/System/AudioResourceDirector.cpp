#include "Project/Audio/System/AudioResourceDirector.hpp"

#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Creates the resource layers.
 * @param layerNum Number of layers.
 * @param itemNumPerLayer Number of sound items each layer can track.
 * @param pLoader Loader used by the layers.
 * @param pHeapController Controller of the sound heap states.
 * @param pPlayer Audio player.
 */
AudioResourceDirector::AudioResourceDirector(s32 layerNum, s32 itemNumPerLayer, IAudioResourceLoader* pLoader,
                                             IAudioHeapController* pHeapController,
                                             const SeadAudioPlayer* pPlayer)
    : mHeapController(pHeapController) {
    mLayers.allocBuffer(layerNum, nullptr);
    for (s32 i = 0; i < layerNum; i++) {
        mLayers.pushBack(new AudioResourceLayer(pLoader, itemNumPerLayer));
    }
}

/**
 * Creates a layer if none with the same name exists.
 * @param rName Layer name.
 * @return True if a layer was created.
 */
bool AudioResourceDirector::tryCreateAudioResourceLayer(const sead::SafeString& rName) {
    for (s32 i = 0; i <= mCurLayerIndex; i++) {
        if (isEqualString(mLayers.unsafeAt(i)->getName().cstr(), rName)) {
            return false;
        }
    }
    createAudioResourceLayer(rName);
    return true;
}

/**
 * Checks whether a layer exists.
 * @param rName Layer name.
 * @return True if the layer exists.
 */
bool AudioResourceDirector::isExistAudioResourceLayer(const sead::SafeString& rName) const {
    for (s32 i = 0; i <= mCurLayerIndex; i++) {
        if (isEqualString(mLayers.unsafeAt(i)->getName().cstr(), rName)) {
            return true;
        }
    }
    return false;
}

/**
 * Creates a new layer on top of the current one and saves the heap state.
 * @param rName Layer name.
 */
void AudioResourceDirector::createAudioResourceLayer(const sead::SafeString& rName) {
    mCurLayerIndex++;
    s32 level = mHeapController->getCurrentHeapStateLevel();
    if (mCurLayerIndex > level) {
        level = mHeapController->saveHeapState();
    }
    if (level < 0) {
        return;
    }
    mLayers.unsafeAt(mCurLayerIndex)->setData(rName);
}

/**
 * Destroys the current layer if it exists.
 * @param rName Layer name.
 * @return True if a layer was destroyed.
 */
bool AudioResourceDirector::tryDestroyAudioResourceLayer(const sead::SafeString& rName) {
    for (s32 i = 0; i <= mCurLayerIndex; i++) {
        if (isEqualString(mLayers.unsafeAt(i)->getName().cstr(), rName)) {
            destroyAudioResourceLayer(rName);
            return true;
        }
    }
    return false;
}

/**
 * Destroys the current layer and restores the heap state.
 * @param rName Layer name.
 */
void AudioResourceDirector::destroyAudioResourceLayer(const sead::SafeString& rName) {
    mHeapController->loadHeapState(mCurLayerIndex);
    mCurLayerIndex--;
}

/**
 * Destroys all layers.
 */
void AudioResourceDirector::destroyAllAudioResourceLayer() {
    while (mCurLayerIndex >= 0) {
        destroyAudioResourceLayer(mLayers.unsafeAt(mCurLayerIndex)->getName().cstr());
    }
}

/**
 * Gets the current layer.
 * @return Current layer.
 */
AudioResourceLayer* AudioResourceDirector::getCurrentAudioResourceLayer() const {
    return mLayers.unsafeAt(mCurLayerIndex);
}

/**
 * Finds a layer by name.
 * @param rName Layer name.
 * @return Layer, or nullptr.
 */
AudioResourceLayer* AudioResourceDirector::findAudioResourceLayer(const sead::SafeString& rName) const {
    for (s32 i = 0; i <= mCurLayerIndex; i++) {
        AudioResourceLayer* layer = mLayers.unsafeAt(i);
        if (isEqualString(layer->getName().cstr(), rName)) {
            return layer;
        }
    }
    return nullptr;
}

/**
 * Checks whether any layer has loaded a sound item.
 * @param id Item id.
 * @return True if loaded.
 */
bool AudioResourceDirector::isLoadedSoundItemAnyLayer(u32 id) const {
    for (s32 i = 0; i <= mCurLayerIndex; i++) {
        if (mLayers.unsafeAt(i)->isLoadedSoundItem(id)) {
            return true;
        }
    }
    return false;
}

/**
 * Loads a sound item into the current layer unless a layer already has it.
 * @param id Item id.
 * @param loadFlag Load flags.
 * @return True if the item is loaded.
 */
bool AudioResourceDirector::loadSoundItem(u32 id, u32 loadFlag) {
    if (isLoadedSoundItemAnyLayer(id)) {
        return true;
    }
    return mLayers.unsafeAt(mCurLayerIndex)->loadSoundItem(id, loadFlag);
}

/**
 * Checks whether any layer has loaded a sound item.
 * @param id Item id.
 * @return True if loaded.
 */
bool AudioResourceDirector::isLoadedSoundItem(u32 id) {
    return isLoadedSoundItemAnyLayer(id);
}
}  // namespace al

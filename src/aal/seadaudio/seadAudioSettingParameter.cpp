#include "audio/seadAudioSettingParameter.h"

namespace sead {
/**
 * Constructs an empty audio setting parameter set.
 */
AudioSettingParameter::AudioSettingParameter() {
    mSubsetList.initOffset(AudioSubsetBase::getListNodeOffset());
}

/**
 * Sets the audio system to use instead of the default one.
 * @param pSystem Audio system.
 */
void AudioSettingParameter::setAudioSystem(AudioSystem* pSystem) {
    mAudioSystem = pSystem;
}

/**
 * Sets the resetter to use instead of the default one.
 * @param pResetter Audio resetter.
 */
void AudioSettingParameter::setResetter(AudioResetter* pResetter) {
    mResetter = pResetter;
}

/**
 * Sets the player to use instead of the default one.
 * @param pPlayer Audio player.
 */
void AudioSettingParameter::setPlayer(AudioPlayer* pPlayer) {
    mPlayer = pPlayer;
}

/**
 * Sets the resource loader.
 * @param pLoader Audio resource loader.
 */
void AudioSettingParameter::setResourceLoader(AudioResourceLoader* pLoader) {
    mResourceLoader = pLoader;
}

/**
 * Adds a subset that the audio manager takes over when it is prepared.
 * @param pSubset Audio subset.
 */
void AudioSettingParameter::appendSubset(AudioSubsetBase* pSubset) {
    mSubsetList.pushBack(pSubset);
}
}  // namespace sead

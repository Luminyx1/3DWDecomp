#include "Project/Audio/System/AudioEffectDataBase.hpp"
#include "Project/Audio/System/AudioSystemDebug.hpp"

#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Audio/AudioBusInfo.hpp"

namespace al {

/**
 * Loads the audio effect database from the given archive.
 * @param pArchiveName Archive path.
 * @param pUnused Unused.
 */
AudioEffectDataBase::AudioEffectDataBase(const char* pArchiveName, const char* pUnused) {
    Resource* resource = findOrCreateResource(pArchiveName, nullptr);
    ByamlIter rootIter(resource->getByml("SeEffectInfoList"));

    {
        ByamlIter iter;
        mEffectBusSettingInfoList = rootIter.tryGetIterByKey(&iter, "EffectBusSettingInfoList") ?
                                        createInfoList<SeEffectBusSettingInfo>(iter) :
                                        nullptr;
    }

    {
        ByamlIter iter;
        mEffectInfoList = rootIter.tryGetIterByKey(&iter, "EffectInfoList") ?
                              createInfoList<SeEffectInfo>(iter) :
                              nullptr;
    }

    {
        ByamlIter iter;
        mStageEffectInfoList = rootIter.tryGetIterByKey(&iter, "StageEffectInfoList") ?
                                   createInfoList<SeStageEffectInfo>(iter) :
                                   nullptr;
    }
}

/**
 * Constructs the audio system debugger.
 */
AudioSystemDebug::AudioSystemDebug() = default;

/**
 * Updates the debug display timer.
 */
void AudioSystemDebug::update() {
    if (mTimer > 0) {
        mTimer--;
    }

    if (mState == 24) {
        mTimer = 30;
    }
}

/**
 * Draws the debug display. Does nothing in release builds.
 */
void AudioSystemDebug::draw() const {}

}  // namespace al

#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>
#include <prim/seadSafeString.h>

#include "Project/Audio/System/AudioResourceLoader.hpp"

namespace al {
class AudioResourceLayer;
class SeadAudioPlayer;

class AudioResourceDirector : public IAudioResourceLoader {
public:
    AudioResourceDirector(s32 layerNum, s32 itemNumPerLayer, IAudioResourceLoader* pLoader,
                          IAudioHeapController* pHeapController, const SeadAudioPlayer* pPlayer);

    bool tryCreateAudioResourceLayer(const sead::SafeString& rName);
    bool isExistAudioResourceLayer(const sead::SafeString& rName) const;
    void createAudioResourceLayer(const sead::SafeString& rName);
    bool tryDestroyAudioResourceLayer(const sead::SafeString& rName);
    void destroyAudioResourceLayer(const sead::SafeString& rName);
    void destroyAllAudioResourceLayer();
    AudioResourceLayer* getCurrentAudioResourceLayer() const;
    AudioResourceLayer* findAudioResourceLayer(const sead::SafeString& rName) const;
    bool isLoadedSoundItemAnyLayer(u32 id) const;
    bool loadSoundItem(u32 id, u32 loadFlag) override;
    bool isLoadedSoundItem(u32 id) override;

private:
    sead::PtrArray<AudioResourceLayer> mLayers;
    s32 mCurLayerIndex = -1;
    IAudioHeapController* mHeapController;
};
static_assert(sizeof(AudioResourceDirector) == 0x28);
}  // namespace al

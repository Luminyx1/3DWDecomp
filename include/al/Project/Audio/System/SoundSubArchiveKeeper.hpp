#pragma once

#include "Project/Audio/System/AudioPlayer.hpp"
#include "Project/Audio/System/AudioResourceLoader.hpp"

namespace al {
class AcLSoundHandle;

class SoundSubArchiveKeeper : public IAudioResourceLoader,
                              public IAudioHeapController,
                              public IUseSeadAudioPlayer {
public:
    SoundSubArchiveKeeper(s32 heapSize);

    void update();

    bool loadSoundItem(u32 id, u32 loadFlag) override;
    bool isLoadedSoundItem(u32 id) override;
    s32 saveHeapState() override;
    void loadHeapState(s32 level) override;
    s32 getCurrentHeapStateLevel() override;
    u64 getSoundResourceHeapFreeSize() override;
    SeadAudioPlayer* getSeadAudioPlayer() const override;

private:
    AcLSoundHandle* mSoundHandle = nullptr;
    SeadAudioPlayer* mAudioPlayer = nullptr;
};
}  // namespace al

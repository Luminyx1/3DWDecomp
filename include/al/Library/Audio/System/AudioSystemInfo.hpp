#pragma once

#include <basis/seadTypes.h>

namespace sead {
class AudioMgr;
}

namespace al {
class AudioEffectDataBase;
class IUseAudioKeeper;
class BgmDataBase;
class SeadAudio3DMgr;
class SeadAudioPlayer;
class SeDataBase;

class AudioSystemInfo {
public:
    AudioSystemInfo();

    SeadAudioPlayer* getSeadAudioPlayerForSe() const;
    SeadAudioPlayer* getSeadAudioPlayerForBgm() const;

    sead::AudioMgr* mAudioMgr = nullptr;
    AudioEffectDataBase* mAudioEffectDataBase = nullptr;
    SeDataBase* mSeDataBase = nullptr;
    BgmDataBase* mBgmDataBase = nullptr;
    SeadAudio3DMgr* mAudio3DMgr = nullptr;
    void* _28 = nullptr;
    void* _30 = nullptr;
    void* _38 = nullptr;
    SeadAudioPlayer* mBgmAudioPlayer = nullptr;
    IUseAudioKeeper* mUpperLayerAudioUser = nullptr;
};

static_assert(sizeof(AudioSystemInfo) == 0x50);
}  // namespace al

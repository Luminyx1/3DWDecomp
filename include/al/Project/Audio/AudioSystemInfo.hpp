#pragma once

#include <basis/seadTypes.h>

namespace al {
class BgmDataBase;
class SeadAudio3DMgr;
class SeadAudioPlayer;

class AudioSystemInfo {
public:
    AudioSystemInfo();

    SeadAudioPlayer* getSeadAudioPlayerForSe() const;
    SeadAudioPlayer* getSeadAudioPlayerForBgm() const;

    void* _0 = nullptr;
    void* _8 = nullptr;
    void* _10 = nullptr;
    BgmDataBase* mBgmDataBase = nullptr;     // _18
    SeadAudio3DMgr* mAudio3DMgr = nullptr;   // _20
    void* _28 = nullptr;
    void* _30 = nullptr;
    void* _38 = nullptr;
    SeadAudioPlayer* mBgmAudioPlayer = nullptr;  // _40
    void* _48 = nullptr;
};
}  // namespace al

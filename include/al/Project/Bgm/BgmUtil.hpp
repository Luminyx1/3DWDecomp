#pragma once

namespace al {
    class IUseAudioKeeper;
    struct BgmPlayingRequest;

    bool tryStopAllBgm(const IUseAudioKeeper*, int);
    void startBgm(const IUseAudioKeeper*, const BgmPlayingRequest&);
    void stopBgm(const IUseAudioKeeper*, const BgmPlayingRequest&);
    void pauseBgm(const IUseAudioKeeper*, const char*, int);
    void resumeBgm(const IUseAudioKeeper*, const char*, int);
    bool isEnableRhythmAnim(const IUseAudioKeeper*, const char*);
    bool isTriggerBeat(const IUseAudioKeeper*, int);
};  // namespace al
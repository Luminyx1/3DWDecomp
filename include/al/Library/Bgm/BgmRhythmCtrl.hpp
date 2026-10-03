#pragma once

#include <basis/seadTypes.h>

#include "Library/Bgm/IUseActiveBgmLine.hpp"

namespace al {
struct BgmChordInfo;
class BgmRhythmCtrl {
public:
    BgmRhythmCtrl(f32 frameRate);

    void init(IUseActiveBgmLine* pActiveBgmLine);
    void update();
    f32 getCurrentBpm() const;
    bool isEnableRhythmAnim() const;
    bool isTriggerRestartBgm() const;
    bool isTriggerBeat(s32 beat) const;
    bool isTriggerBeatForAnime(s32 beat) const;
    bool isTriggerRhythm() const;
    bool isTriggerAnimChange() const;
    s32 getAnimType() const;
    f32 getAnimFrame() const;
    f32 getBeatRate() const;
    f32 getBeatRateForAnime() const;
    const BgmChordInfo* getChordInfoCurrent() const;
    f32 getCurBeat() const;
    f32 getBeatPerFrame() const;
    f32 getFrameRate() const;

private:
    IUseActiveBgmLine* mActiveBgmLine = nullptr;
    f32 mFrameRate;
};

static_assert(sizeof(BgmRhythmCtrl) == 0x10);
}  // namespace al

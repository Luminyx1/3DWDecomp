#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>

namespace al {
class BgmMusicalInfo;
class ByamlIter;
struct BgmChordInfo;
struct BgmRhythmInfo;

class BgmRhythmDetector {
public:
    BgmRhythmDetector();

    void init(BgmMusicalInfo* pMusicalInfo, f32 bpm, s32 sampleRate, s32 beginSample, s32 loopStartSample);
    s32 getTrgStartSample() const;
    void update(s32 curSample);
    const BgmRhythmInfo* tryFindCurRhythmInfo(f32 beat) const;
    const BgmRhythmInfo* tryFindNextRhythmInfo(f32 beat) const;
    bool isOverBeatInRhythmInfoList(f32 beat) const;
    bool isTriggerBeat(s32 beat) const;
    bool isTriggerBeatForAnime(s32 beat) const;
    f32 calcAnimFrame(f32 beat) const;
    s32 findAnimType(f32 beat) const;
    void initBeatList(ByamlIter iter);
    void initChordList(ByamlIter iter);

    f32 getAnimFrame() const { return mAnimFrame; }
    s32 getAnimType() const { return mAnimType; }
    bool isTriggerRestartBgm() const { return mIsTriggerRestartBgm; }
    bool isTriggerRhythm() const { return mIsTriggerRhythm; }
    bool isTriggerAnimChange() const { return mIsTriggerAnimChange; }
    const BgmChordInfo* getChordInfoCurrent() const { return mChordInfoCurrent; }
    f32 getBeatPerFrame() const { return mBeatPerFrame; }
    f32 getFrameRate() const { return mFrameRate; }
    f32 getBeatRate() const { return mBeatRate; }
    f32 getBeatRateForAnime() const { return mBeatRateForAnime; }
    f32 getCurBeat() const { return mCurBeat; }

private:
    s32 _0 = -1;
    s32 _4 = -1;
    f32 mAnimFrame = 0.0f;
    s32 mAnimType = 0;
    bool _10 = true;
    bool _11 = false;
    bool _12 = false;
    bool mIsTriggerRestartBgm = true;
    bool _14 = false;
    bool _15 = false;
    bool mIsTriggerRhythm = false;
    bool mIsTriggerAnimChange = false;
    const BgmChordInfo* mChordInfoCurrent = nullptr;
    sead::PtrArray<s32> _20;
    f32 mBpm = 1.0f;
    f32 _34 = 0.0f;
    BgmMusicalInfo* mMusicalInfo = nullptr;
    bool _40 = false;
    f32 _44 = 0.0f;
    f32 mBeatPerFrame = 0.0f;
    f32 mFrameRate = 0.0f;
    f32 _50 = 0.0f;
    f32 _54 = 0.0f;
    f32 mBeatRate = 0.0f;
    f32 mBeatRateForAnime = 0.0f;
    u8 _60[0x8] = {};
    f32 _68 = 0.0f;
    f32 mCurBeat = 0.0f;
    f32 _70 = 0.0f;
    f32 _74 = 0.0f;
    f32 _78 = 0.0f;
    s32 mSampleRate = 32000;
};
static_assert(sizeof(BgmRhythmDetector) == 0x80);
}  // namespace al

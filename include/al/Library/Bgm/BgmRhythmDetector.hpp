#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>

namespace al {
struct BgmMusicalInfo;
class ByamlIter;
struct BgmChordInfo;
struct BgmRhythmInfo;

class BgmRhythmDetector {
public:
    BgmRhythmDetector();

    void init(BgmMusicalInfo* pMusicalInfo, f32 bpm, s32 sampleRate, s32 beginSample,
              s32 loopStartSample);
    s32 getTrgStartSample() const;
    void update(s32 curSample);
    const BgmRhythmInfo* tryFindCurRhythmInfo(f32 beat) const;
    const BgmRhythmInfo* tryFindNextRhythmInfo(f32 beat) const;
    bool isOverBeatInRhythmInfoList(f32 beat) const;
    bool isTriggerBeat(s32 beat) const;
    bool isTriggerBeatForAnime(s32 beat) const;
    s32 calcAnimFrame(f32 beat) const;
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
    bool mIsFirstUpdate = true;
    bool mIsStartBeat = false;
    bool mIsStartBeatForAnime = false;
    bool mIsTriggerRestartBgm = true;
    bool mIsTriggerBeat = false;
    bool mIsTriggerBeatForAnime = false;
    bool mIsTriggerRhythm = false;
    bool mIsTriggerAnimChange = false;
    const BgmChordInfo* mChordInfoCurrent = nullptr;
    sead::PtrArray<s32> _20;
    f32 mBpm = 1.0f;
    f32 mBeatOffset = 0.0f;
    BgmMusicalInfo* mMusicalInfo = nullptr;
    bool _40 = false;
    f32 mSamplePerBeat = 0.0f;
    f32 mBeatPerFrame = 0.0f;
    f32 mFrameRate = 0.0f;
    f32 _50 = 0.0f;
    f32 mCurRhythmBeat = 0.0f;
    f32 mBeatRate = 0.0f;
    f32 mBeatRateForAnime = 0.0f;
    const BgmRhythmInfo* mNextRhythmInfo = nullptr;
    bool mIsOverRhythmInfoList = false;
    f32 mCurBeat = 0.0f;
    f32 mCurBeatForAnime = 0.0f;
    s32 mBeatCount = 0;
    s32 mBeatCountForAnime = 0;
    s32 mSampleRate = 32000;
};

static_assert(sizeof(BgmRhythmDetector) == 0x80);
}  // namespace al

#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>

#include "Project/Audio/AudioInfoList.hpp"

namespace al {
class AudioSystemInfo;
class Bgm;
class BgmLineInfo;
struct BgmMusicalInfo;
class BgmPlayInfo;
class BgmProcInfo;
class BgmResourceInfo;
class BgmRhythmDetector;
class BgmEnableSituationInfo;
class BgmSituationInfo;
class BgmStagePlayInfo;
struct BgmPlayingRequest;

/**
 * A playable BGM entry of a line: the play name together with its resource and musical data.
 */
struct BgmLinePlayInfo {
    const char* name = nullptr;
    const BgmResourceInfo* resourceInfo = nullptr;
    BgmMusicalInfo* musicalInfo = nullptr;
    s32 startDelayFrames = 0;
    s32 fadeInFrames = 0;
};

static_assert(sizeof(BgmLinePlayInfo) == 0x20);

using BgmLinePlayInfoArray = sead::PtrArray<BgmLinePlayInfo>;

class BgmLine {
public:
    enum State : u32 {
        State_WaitStart = 0,
        State_Play = 1,
        State_Pause = 2,
        State_PauseFadeOut = 3,
        State_FadeOut = 4,
        State_None = 5,
    };

    BgmLine(f32 bpmRate);

    void init(AudioSystemInfo* pInfo, const BgmLineInfo* pLineInfo, const char* pStageName);
    void update();
    void clearBgmLine();
    bool isRunning() const;
    bool isPrepared() const;
    bool isEnableRhythmDetection() const;
    void startBgm(const BgmPlayingRequest& rRequest);
    bool isRunningByPlayName(const char* pName) const;
    bool isFadeOut() const;
    bool isPreparedByPlayName(const char* pName) const;
    void startPreparedBgm(const BgmPlayingRequest& rRequest);
    void stopBgm(s32 fadeFrames);
    void changeSituation(const char* pName, bool isForce);
    const BgmEnableSituationInfo* tryGetBgmEnableSituationInfo(const char* pName);
    void prepareBgm(const BgmPlayingRequest& rRequest);
    bool isUnnecessaryPrepare(const char* pName) const;
    void startWaitingBgm();
    s32 startIslandBgm(const BgmPlayingRequest& rRequest, s32 startSample, s32 startDelayFrames,
                       s32 fadeOutFrames);
    void pauseBgm(s32 fadeFrames);
    bool isPause() const;
    void resumeBgm(s32 fadeFrames);
    void stopAllBgmPlayer(s32 fadeFrames);
    void attachSuffix(const BgmProcInfo* pInfo, bool isForce);
    void detachSuffix(const BgmProcInfo* pInfo);
    bool isWaitStart() const;
    const char* getCurPlayName() const;
    const char* getLineName() const;
    f32 getCurBpm() const;
    void changePitch(f32 pitch);
    s32 getCurPlayPos() const;
    void changeBgmVolume(f32 volume, s32 fadeFrames);
    u32 getUnactiveBgmPlayerIndex() const;

    const char* getSituationName() const { return mSituationName; }

    BgmRhythmDetector* getRhythmDetector() const { return mRhythmDetector; }

    void setIsDisableAutoStop(bool isDisable) { mIsDisableAutoStop = isDisable; }

private:
    Bgm* getCurBgm() const { return mBgmPlayers[mCurPlayerIndex]; }

    BgmLinePlayInfo* findPlayInfo(const char* pName) const;
    bool isEnableSituation(const char* pName) const;
    void startTriggerSituation();
    void startSituationAfterStart();

    const BgmLineInfo* mLineInfo = nullptr;
    AudioInfoList<BgmPlayInfo>* mPlayInfoList = nullptr;
    AudioInfoList<BgmStagePlayInfo>* mStagePlayInfoList = nullptr;
    Bgm** mBgmPlayers = nullptr;
    u32 mCurPlayerIndex = 0;
    f32 mBpmRate;
    BgmLinePlayInfoArray* mPlayInfos = nullptr;
    BgmRhythmDetector* mRhythmDetector = nullptr;
    AudioInfoList<BgmSituationInfo>* mSituationInfoList = nullptr;
    BgmLinePlayInfo* mCurPlayInfo = nullptr;
    BgmLinePlayInfo* mPreparedPlayInfo = nullptr;
    State mState = State_None;
    bool mIsHurry = false;
    bool mIsInWater = false;
    bool mIsDisableAutoStop = false;
    const char* mSituationName = nullptr;
};

static_assert(sizeof(BgmLine) == 0x60);
}  // namespace al

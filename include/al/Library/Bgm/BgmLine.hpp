#pragma once

#include <basis/seadTypes.h>

namespace al {
class AudioSystemInfo;
class BgmLineInfo;
class BgmProcInfo;
class BgmRhythmDetector;
class BgmEnableSituationInfo;
struct BgmPlayingRequest;

class BgmLine {
public:
    BgmLine(f32 frameRate);

    void init(AudioSystemInfo* pInfo, const BgmLineInfo* pLineInfo, const char* pUserName);
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
    void startIslandBgm(const BgmPlayingRequest& rRequest, s32 unk1, s32 unk2, s32 unk3);
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
    s32 getUnactiveBgmPlayerIndex() const;

    const char* getSituationName() const { return mSituationName; }
    BgmRhythmDetector* getRhythmDetector() const { return mRhythmDetector; }

private:
    u8 _0[0x30];
    BgmRhythmDetector* mRhythmDetector;
    u8 _38[0x20];
    const char* mSituationName;
};
}  // namespace al

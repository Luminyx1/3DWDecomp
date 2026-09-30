#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class BgmDirector;
class IUseAudioKeeper;
class LiveActor;
struct BgmPlayingRequest;

BgmDirector* getBgmDirector(const IUseAudioKeeper* pUser);
BgmDirector* tryGetBgmDirector(const IUseAudioKeeper* pUser);
IUseAudioKeeper* getUpperLayerAudioUser(const IUseAudioKeeper* pUser);
IUseAudioKeeper* tryGetUpperLayerAudioUser(const IUseAudioKeeper* pUser);
BgmDirector* getActiveBgmDirector(const IUseAudioKeeper* pUser);
BgmDirector* tryGetActiveBgmDirector(const IUseAudioKeeper* pUser);
const char* getCurPlayingBgmPlayName(const IUseAudioKeeper* pUser);
void startSequenceBgm(const IUseAudioKeeper* pUser, const char* pName, s32 fadeInFrames, s32 delayFrames);
void startBgm(const IUseAudioKeeper* pUser, const char* pName, s32 fadeInFrames, s32 delayFrames,
              s32 fadeOutFrames, s32 unk);
void startSequenceBgm(const IUseAudioKeeper* pUser, const BgmPlayingRequest& rRequest);
void startBgm(const IUseAudioKeeper* pUser, const BgmPlayingRequest& rRequest);
void stopSequenceBgm(const IUseAudioKeeper* pUser, const char* pName, s32 fadeOutFrames);
void stopBgm(const IUseAudioKeeper* pUser, const char* pName, s32 fadeOutFrames, s32 unk);
void stopSequenceBgm(const IUseAudioKeeper* pUser, const BgmPlayingRequest& rRequest);
void stopBgm(const IUseAudioKeeper* pUser, const BgmPlayingRequest& rRequest);
void stopActiveSequenceBgm(const IUseAudioKeeper* pUser, s32 fadeOutFrames);
void prepareSequenceBgm(const IUseAudioKeeper* pUser, const BgmPlayingRequest& rRequest);
void prepareBgm(const IUseAudioKeeper* pUser, const BgmPlayingRequest& rRequest);
void startSequencePreparedBgm(const IUseAudioKeeper* pUser, const char* pName);
void startPreparedBgm(const IUseAudioKeeper* pUser, const char* pName);
void startSequenceBgmWithAreaCheck(const IUseAudioKeeper* pUser, bool isIgnoreDefault, s32 fadeInFrames,
                                   s32 delayFrames, s32 fadeOutFrames);
void changeBgmSituation(const IUseAudioKeeper* pUser, const char* pName);
void pauseSequenceBgm(const IUseAudioKeeper* pUser, const char* pName, s32 fadeFrames);
void pauseBgm(const IUseAudioKeeper* pUser, const char* pName, s32 fadeFrames);
void resumeSequenceBgm(const IUseAudioKeeper* pUser, const char* pName, s32 fadeFrames);
void resumeBgm(const IUseAudioKeeper* pUser, const char* pName, s32 fadeFrames);
void prepareBgmWithAreaCheck(const IUseAudioKeeper* pUser);
void startBgmWithAreaCheck(const IUseAudioKeeper* pUser, bool isIgnoreDefault, s32 fadeInFrames, s32 delayFrames,
                           s32 fadeOutFrames);
void pauseActiveBgm(const IUseAudioKeeper* pUser, s32 fadeFrames);
void resumeActiveBgm(const IUseAudioKeeper* pUser, s32 fadeFrames);
void pauseIslandBgm(const IUseAudioKeeper* pUser, s32 fadeFrames);
void resumeIslandBgm(const IUseAudioKeeper* pUser, s32 fadeFrames);
void pauseOceanBgm(const IUseAudioKeeper* pUser, s32 fadeFrames);
void resumeOceanBgm(const IUseAudioKeeper* pUser, s32 fadeFrames);
bool isPauseActiveBgm(const IUseAudioKeeper* pUser);
bool isPauseBgm(const IUseAudioKeeper* pUser, const char* pName);
void stopAllBgm(const IUseAudioKeeper* pUser, s32 fadeOutFrames);
void stopAllSequenceBgm(const IUseAudioKeeper* pUser, s32 fadeOutFrames);
void tryStopAllBgm(const IUseAudioKeeper* pUser, s32 fadeOutFrames);
bool isBgmCurrentlyPlaying(const IUseAudioKeeper* pUser, const char* pName);
void tryPauseBgmIfDifferBgmArea(const LiveActor* pActor, const sead::Vector3f& rPos, s32 fadeFrames);
void disableChangeSituation(const IUseAudioKeeper* pUser);
void enableChangeSituation(const IUseAudioKeeper* pUser);
const char* getBgmLineSituationName(const IUseAudioKeeper* pUser, const char* pLineName);
bool isEqualBgmLineSituationName(const IUseAudioKeeper* pUser, const char* pLineName, const char* pName);
bool isEqualBgmActiveLineSituationName(const IUseAudioKeeper* pUser, const char* pName);
void disableBgmChangeArea(const IUseAudioKeeper* pUser);
void enableBgmChangeArea(const IUseAudioKeeper* pUser);
void startBgmSyncedCurBgmBeat(const IUseAudioKeeper* pUser, const BgmPlayingRequest& rRequest, s32 unk);
void stopBgmSyncedCurBgmBeat(const IUseAudioKeeper* pUser, const BgmPlayingRequest& rRequest, s32 unk);
void changeLineAutoStopMode(const IUseAudioKeeper* pUser, const char* pLineName, bool isAutoStop);
void disableLineChange(const IUseAudioKeeper* pUser, bool isDisable);
void disableBgmStart(const IUseAudioKeeper* pUser);
void enableBgmStart(const IUseAudioKeeper* pUser);
void changeBgmVolume(const IUseAudioKeeper* pUser, f32 volume, s32 frames);
void disableVolumeChange(const IUseAudioKeeper* pUser);
void enableVolumeChange(const IUseAudioKeeper* pUser);
void changeIslandMapBgmVolume(const IUseAudioKeeper* pUser, s32 unk1, s32 unk2, bool unk3);
s32 getBgmSamplePos(const IUseAudioKeeper* pUser, const char* pName);
void tryPrepareActionFirstBgm(const LiveActor* pActor, const char* pActionName);
void setActiveBgmPitch(const IUseAudioKeeper* pUser, f32 pitch);
bool isEnableRhythmAnim(const IUseAudioKeeper* pUser, const char* pName);
bool isTriggerRestartBgm(const IUseAudioKeeper* pUser);
bool isTriggerBeat(const IUseAudioKeeper* pUser, s32 beat);
bool isTriggerBeatForAnime(const IUseAudioKeeper* pUser, s32 beat);
bool isTriggerRhythm(const IUseAudioKeeper* pUser);
bool isTriggerRhythmAnimChange(const IUseAudioKeeper* pUser);
s32 getRhythmAnimType(const IUseAudioKeeper* pUser);
f32 getRhythmAnimFrame(const IUseAudioKeeper* pUser);
f32 getBeatRate(const IUseAudioKeeper* pUser);
f32 getBeatRateForAnime(const IUseAudioKeeper* pUser);
f32 getCurBeat(const IUseAudioKeeper* pUser);
f32 getBeatPerFrame(const IUseAudioKeeper* pUser);
f32 getFrameRate(const IUseAudioKeeper* pUser);
void pauseOnBgm(const IUseAudioKeeper* pUser);
void pauseOffBgm(const IUseAudioKeeper* pUser);
void muteOnRunningLineTrack(IUseAudioKeeper* pUser, u32 track, bool unk);
void muteOffRunningLineTrack(IUseAudioKeeper* pUser, u32 track, bool unk);
}  // namespace al

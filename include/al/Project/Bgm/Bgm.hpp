#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>

#include "Project/Audio/AudioInfoList.hpp"

namespace sead {
class SoundHandle;
}

namespace al {
class AudioSystemInfo;
class Bgm;
class BgmLpfController;
class BgmPitchController;
class BgmVolumeController;
class BgmResourceInfo;
class BgmResourceSuffixInfo;
class BgmTrackChangeInfo;
class BgmSuffixProcInfo;
class BgmVolumeProcInfo;
class BgmTrackProcInfo;
class BgmRegionProcInfo;
class BgmPitchProcInfo;
class BgmPitchModulationProcInfo;
class BgmLpfProcInfo;
class SeadAudioPlayer;
struct BgmPlayingRequest;

using BgmRegionProcInfoArray = sead::PtrArray<const BgmRegionProcInfo>;
using BgmTrackChangeInfoList = AudioInfoList<BgmTrackChangeInfo>;

/**
 * Tracks the stream region jumps requested by region procs of a BGM.
 */
class BgmRegionCtrl {
public:
    /**
     * Constructs a region controller with a buffer for the played region procs.
     */
    BgmRegionCtrl() {
        mPlayedRegionInfos = new BgmRegionProcInfoArray();
        mPlayedRegionInfos->allocBuffer(20, nullptr);
    }

    /**
     * Checks whether a region proc already ran once.
     * @param pInfo Region proc.
     * @return True if the region proc already ran.
     */
    bool isPlayedRegion(const BgmRegionProcInfo* pInfo) const {
        for (s32 i = 0; i < mPlayedRegionInfos->size(); i++) {
            if (mPlayedRegionInfos->unsafeAt(i) == pInfo) {
                return true;
            }
        }

        return false;
    }

    Bgm* mBgm;
    const BgmRegionProcInfo* mRegionInfo;
    BgmRegionProcInfoArray* mPlayedRegionInfos;
    s32 mRegionNo = 0;
    bool mIsChangedRegion = false;
    bool mIsRestoreRegionNo = false;
};

static_assert(sizeof(BgmRegionCtrl) == 0x20);

/**
 * Parameters used to start the sound of a BGM.
 */
struct BgmSoundStartParam {
    void set(const BgmResourceInfo* pResourceInfo, const char* pName, s32 startSample, s32 fadeInFrames,
             s32 fadeOutFrames, BgmRegionCtrl* pRegionCtrl);

    const char* mName = nullptr;
    s32 mStartSample = 0;
    s32 mFadeInFrames = 0;
    s32 mFadeOutFrames = 0;
    BgmRegionCtrl* mRegionCtrl = nullptr;
    bool mIsDisableAudioEffect = false;
    bool mIsEnableNwRender = false;
};

static_assert(sizeof(BgmSoundStartParam) == 0x28);

class Bgm {
public:
    Bgm();

    void init(AudioSystemInfo* pInfo);
    void update();
    void startBgm(const BgmResourceInfo* pResourceInfo, const BgmPlayingRequest& rRequest, bool isPrepare);
    void startPreparedBgm(const BgmPlayingRequest& rRequest);
    void startPreparedBgmExistingRequest();
    void pauseBgm(s32 fadeFrames);
    void resumeBgm(s32 fadeFrames);
    void stopBgm(s32 fadeFrames);
    bool isFadeOutNow() const;
    bool isFadeInNow() const;
    f32 getCurBpm() const;
    s32 getCurSamplePosition() const;
    bool tryGetCurSamplePosition(s32* pPosition);
    void changeVolume(const BgmVolumeProcInfo* pInfo);
    void changeTrack(const BgmTrackProcInfo* pInfo);
    void changeRegion(const BgmRegionProcInfo* pInfo);
    void changePitch(const BgmPitchProcInfo* pInfo);
    void changePitch(f32 pitch, f32 speed);
    void modulatePitch(const BgmPitchModulationProcInfo* pInfo);
    void lpf(const BgmLpfProcInfo* pInfo);
    void movePlayPosition(s32 sample);
    s32 attachSuffix(const BgmSuffixProcInfo* pInfo, bool isStartHead);
    s32 detachSuffix(const BgmSuffixProcInfo* pInfo);

    sead::SoundHandle* getSoundHandle() const { return mSoundHandle; }

    s32 getStartDelayFrames() const { return mStartDelayFrames; }

    bool isPausedInDelay() const { return mIsPausedInDelay; }

    BgmVolumeController* getVolumeController() const { return mVolumeController; }

private:
    bool isPlaying() const;
    bool isPause() const;
    s32 getCurStartSample() const;
    s32 calcCurSamplePositionByBpm(f32 bpm) const;
    BgmRegionCtrl* getEnableRegionCtrl() const;

    const char* mSuffixName = nullptr;
    SeadAudioPlayer* mAudioPlayer = nullptr;
    sead::SoundHandle* mSoundHandle = nullptr;
    const BgmResourceInfo* mResourceInfo = nullptr;
    const BgmResourceSuffixInfo* mResourceSuffixInfo = nullptr;
    f32 mBpm = 0.0f;
    BgmRegionCtrl* mRegionCtrl = nullptr;
    bool mIsPrepared = false;
    s32 mStartDelayFrames = 0;
    bool mIsPausedInDelay = false;
    BgmSoundStartParam* mStartParam;
    BgmVolumeController* mVolumeController;
    BgmLpfController* mLpfController;
    BgmPitchController* mPitchController;
    const BgmTrackChangeInfoList* mTrackChangeInfoList = nullptr;
};

static_assert(sizeof(Bgm) == 0x70);
}  // namespace al

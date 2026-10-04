#pragma once

#include <basis/seadTypes.h>
#include <nn/os.h>
#include <prim/seadSafeString.h>

#include "Project/Audio/System/AudioPlayer.hpp"
#include "Project/Audio/System/AudioResourceLoader.hpp"

namespace aal {
class AudioFrameProcessMgr;
class IAudioFrameProcess;
}  // namespace aal

namespace sead {
class AudioMgr;
class AudioSettingParameter;
}  // namespace sead

namespace al {
class AudioEffectDataBase;
class AudioMic;
class AudioResourceDirector;
class AudioSystemDebug;
class AudioSystemInfo;
class BgmDataBase;
class SeadAudio3DMgr;
class SeadAudioSoundHeapPtrWrapper;
class SeDataBase;
class SoundSubArchiveKeeper;

extern const char* UMF_SE_STATIONED_SYSTEM;
extern const char* UMF_BGM_STATIONED_1ST;
extern const char* UMF_SE_STATIONED_1ST;
extern const char* UMF_SE_STATIONED_2ND;
extern const char* UMF_BGM_STATIONED_2ND;

struct AudioSystemInitInfo {
    const char* archiveName = nullptr;
    bool isUseMic = false;
    f32 masterVolume = 1.0f;
    f32 tvOutputVolume = 1.0f;
    f32 _14 = 1.0f;
    f32 otherOutputVolume = 1.0f;
};

static_assert(sizeof(AudioSystemInitInfo) == 0x20);

class AudioSystem : public IAudioResourceLoader,
                    public IAudioHeapController,
                    public IUseSeadAudioPlayer {
public:
    AudioSystem();
    ~AudioSystem() override;

    void init(const sead::SafeString& rArchiveName, f32 masterVolume, bool isUseMic);
    void init(const AudioSystemInitInfo& rInfo);
    SeadAudio3DMgr* initSeadAudio3DMgr(sead::AudioSettingParameter* pParam);
    void initDebugModule(sead::AudioSettingParameter* pParam);
    SeadAudioPlayer* getSubArchiveSeadAudioPlayer() const;
    void initSpy();
    void initDataBase();
    void initResourceDirector();
    void initSystemInfo(SeadAudio3DMgr* pAudio3DMgr);
    void updateSoundChannelSetting();
    void applyDeviceVolume();
    void update();
    void pauseSystemImmediately(bool isPause, const char* pName, bool isForce);
    bool loadSoundItem(u32 id, u32 loadFlag) override;
    bool isLoadedSoundItem(u32 id) override;
    s32 saveHeapState() override;
    void loadHeapState(s32 level) override;
    s32 getCurrentHeapStateLevel() override;
    u64 getSoundResourceHeapFreeSize() override;
    SeadAudioPlayer* getSeadAudioPlayer() const override;
    void addAudiioFrameProccess(aal::IAudioFrameProcess* pProcess);
    void removeAudiioFrameProccess(aal::IAudioFrameProcess* pProcess);
    void reRegisterCallback(aal::IAudioFrameProcess* pProcess);

    AudioSystemInfo* getAudioSystemInfo() const { return mAudioSystemInfo; }

private:
    sead::AudioMgr* mAudioMgr = nullptr;
    AudioEffectDataBase* mAudioEffectDataBase = nullptr;
    SeDataBase* mSeDataBase = nullptr;
    BgmDataBase* mBgmDataBase = nullptr;
    AudioSystemInfo* mAudioSystemInfo = nullptr;
    AudioResourceDirector* mResourceDirector = nullptr;
    AudioResourceDirector* mSubArchiveResourceDirector = nullptr;
    SeadAudioSoundHeapPtrWrapper* mSoundHeapPtrWrapper = nullptr;
    AudioMic* mAudioMic = nullptr;
    SoundSubArchiveKeeper* mSubArchiveKeeper = nullptr;
    AudioSystemDebug* mAudioSystemDebug = nullptr;
    f32 mMasterVolume = 1.0f;
    f32 mTvOutputVolume = 1.0f;
    f32 mOtherOutputVolume = 1.0f;
    void* _80;
    aal::AudioFrameProcessMgr* mAudioFrameProcessMgr = nullptr;
    nn::os::SystemEvent mDeviceNotificationEvent;
    bool mIsStereo = false;
};

static_assert(sizeof(AudioSystem) == 0xc8);
}  // namespace al

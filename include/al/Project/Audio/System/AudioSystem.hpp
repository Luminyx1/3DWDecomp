#pragma once

#include <basis/seadTypes.h>
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
class AudioResourceDirector;
class AudioSystemDebug;
class AudioSystemInfo;
class BgmDataBase;
class SeadAudio3DMgr;
class SeDataBase;
class SoundSubArchiveKeeper;
struct AudioSystemInitInfo;

class AudioSystem : public IAudioResourceLoader, public IAudioHeapController, public IUseSeadAudioPlayer {
public:
    AudioSystem();

    void init(const sead::SafeString& rArchiveName, f32 volume, bool isUseSubArchive);
    void init(const AudioSystemInitInfo& rInfo);
    void initSeadAudio3DMgr(sead::AudioSettingParameter* pParam);
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
    void* _40 = nullptr;
    void* _48 = nullptr;
    void* _50 = nullptr;
    void* _58 = nullptr;
    SoundSubArchiveKeeper* mSubArchiveKeeper = nullptr;
    AudioSystemDebug* mAudioSystemDebug = nullptr;
    f32 _70[3] = {1.0f, 1.0f, 1.0f};
    void* _80 = nullptr;
    aal::AudioFrameProcessMgr* mAudioFrameProcessMgr = nullptr;
    u8 _90[0x28];
    bool _b8 = false;
    bool _c0 = false;
};
}  // namespace al

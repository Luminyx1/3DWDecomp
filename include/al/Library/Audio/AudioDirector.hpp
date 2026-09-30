#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Project/AreaObj/IUseAreaObj.hpp"

namespace sead {
class PerspectiveProjection;
}

namespace al {
class AudioEventController;
class AudioMic;
class AudioRequestKeeperSyncedBgm;
class AudioSituationDirector;
class AudioSystemInfo;
class AudioVolumeCtrl;
class BgmDirector;
class IUseAudioKeeper;
class PlayerHolder;
class SeDirector;
class SeEffectController;

class AudioDirector : public IUseAreaObj {
public:
    AudioDirector();

    void init(AudioSystemInfo* pInfo, const char* pStageName, s32 seRequestNum, s32 unused1, s32 unused2,
              const char* pBgmStageName, s32 unused3, f32 volume);
    void init3D(const AudioSystemInfo* pInfo, const sead::Vector3f* pCameraPos, const sead::Matrix34f* pCameraMtx,
                const sead::PerspectiveProjection* pProjection, const sead::Vector3f* pCameraAt,
                const char* pStageName, AreaObjDirector* pAreaObjDirector, bool isUseListenerPoser);
    void initAfterInitPlacement(const AudioSystemInfo* pInfo);
    void initSituationDirector(const char** pCategoryNames, s32 categoryNum);
    void initMic(AudioMic* pMic);
    virtual void update();
    virtual void finalize();
    void setPlayerHolder(const PlayerHolder* pPlayerHolder);
    void disableBgmChangeArea();
    void enableBgmChangeArea();
    void setDefaultBgmPlayName(const char* pName);
    void setBgmChangeWatcher(s32 watcher);
    void setOverrideFadeInFrames();

    AreaObjDirector* getAreaObjDirector() const override { return mAreaObjDirector; }

    SeDirector* getSeDirector() const { return mSeDirector; }
    BgmDirector* getBgmDirector() const { return mBgmDirector; }
    AudioVolumeCtrl* getAudioVolumeCtrl() const { return mAudioVolumeCtrl; }
    AudioMic* getAudioMic() const { return mAudioMic; }
    AudioEventController* getAudioEventController() const { return mAudioEventController; }
    AudioRequestKeeperSyncedBgm* getAudioRequestKeeperSyncedBgm() const { return mAudioRequestKeeperSyncedBgm; }
    AudioSystemInfo* getAudioSystemInfo() const { return mAudioSystemInfo; }
    AudioSituationDirector* getAudioSituationDirector() const { return mAudioSituationDirector; }
    SeEffectController* getSeEffectController() const { return mSeEffectController; }
    IUseAudioKeeper* getUpperLayerAudioUser() const { return mUpperLayerAudioUser; }
    bool isForceInvalidSe() const { return _50; }

private:
    SeDirector* mSeDirector = nullptr;
    BgmDirector* mBgmDirector = nullptr;
    AudioVolumeCtrl* mAudioVolumeCtrl = nullptr;
    AudioMic* mAudioMic = nullptr;
    AudioEventController* mAudioEventController = nullptr;
    AudioRequestKeeperSyncedBgm* mAudioRequestKeeperSyncedBgm = nullptr;
    AudioSystemInfo* mAudioSystemInfo = nullptr;
    AreaObjDirector* mAreaObjDirector = nullptr;
    AudioSituationDirector* mAudioSituationDirector = nullptr;
    bool _50 = false;
    SeEffectController* mSeEffectController = nullptr;
    IUseAudioKeeper* mUpperLayerAudioUser = nullptr;
};
static_assert(sizeof(AudioDirector) == 0x68);
}  // namespace al

#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Project/AreaObj/IUseAreaObj.hpp"
#include "Project/Audio/IUseAudioKeeper.hpp"

namespace al {
class AudioDirector;
class AudioGeneralPurposeAreaChecker;
class AudioSituationDirector;
class PlayerHolder;
class SeAreaTriggeredPlayer;
class SeDirector;

class AudioEventController : public IUseAudioKeeper, public IUseAreaObj {
public:
    static const s32 EVENT_PLAY_SE;
    static const s32 EVENT_CHANGE_AUIO_EFFECT;

    AudioEventController(const AudioDirector* pDirector, const char* pDefaultAudioEffectName);

    void init3D(AreaObjDirector* pAreaObjDirector, AudioSituationDirector* pSituationDirector);
    void initAfterInitPlacement(const AudioDirector* pDirector);
    void update();
    bool isEnableAudioEvent(s32 type);
    void finalize();
    void setPlayerHolder(const PlayerHolder* pPlayerHolder);
    void activate();
    void deactivate();
    void activateEachAudioEvent(s32 type);
    void deactivateEachAudioEvent(s32 type);
    bool isInBgmStopArea();
    const char* getBgmPlayNameByAreaChecker(bool isIgnoreDefault);
    const char* getBgmSituationNameByAreaChecker();
    const char* getAudioEffectNameByAreaChecker();
    const char* getBgmPlayNameInThisPosition(const sead::Vector3f& rPos);

    AudioKeeper* getAudioKeeper() const override { return mAudioKeeper; }
    AreaObjDirector* getAreaObjDirector() const override { return mAreaObjDirector; }

    void setIsDisableBgmChangeArea(bool isDisable) { mIsDisableBgmChangeArea = isDisable; }
    void setDefaultBgmPlayName(const char* pName) {
        mDefaultBgmPlayName = pName;
        mCurBgmPlayName = pName;
    }

    void setBgmChangeWatcher(s32 watcher) { mBgmChangeWatcher = watcher; }
    void setIsOverrideFadeInFrames(bool isOverride) { mIsOverrideFadeInFrames = isOverride; }

private:
    AudioKeeper* mAudioKeeper = nullptr;
    AudioGeneralPurposeAreaChecker* mBgmChangeAreaChecker = nullptr;
    AudioGeneralPurposeAreaChecker* mBgmStartAreaChecker = nullptr;
    AudioGeneralPurposeAreaChecker* mBgmStopAreaChecker = nullptr;
    AudioGeneralPurposeAreaChecker* mBgmRegionChangeAreaChecker = nullptr;
    AudioGeneralPurposeAreaChecker* mAudioEffectChangeAreaChecker = nullptr;
    AudioGeneralPurposeAreaChecker* mAudioListenerParamAreaChecker = nullptr;
    AudioGeneralPurposeAreaChecker* mAudioSituationAreaChecker = nullptr;
    SeAreaTriggeredPlayer* mSeAreaTriggeredPlayer = nullptr;
    const PlayerHolder* mPlayerHolder = nullptr;
    AreaObjDirector* mAreaObjDirector = nullptr;
    const char* mCurBgmPlayName = "Stage";
    bool mIsInWater = false;
    bool mIsDisableBgmChangeArea = false;
    s32 mEnableEventFlags = 0;
    s32 mBgmChangeWatcher = -1;
    SeDirector* mSeDirector = nullptr;
    const char* mDefaultBgmPlayName = "Stage";
    const char* mDefaultAudioEffectName;
    AudioSituationDirector* mAudioSituationDirector = nullptr;
    u32 mInWaterFrames = 0;
    bool mIsOverrideFadeInFrames = false;
    s32 mOverrideFadeInFrames = -1;
};

static_assert(sizeof(AudioEventController) == 0xb0);
}  // namespace al

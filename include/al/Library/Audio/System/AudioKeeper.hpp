#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>

#include "Project/AreaObj/IUseAreaObj.hpp"

namespace al {
class AreaObj;
class AudioDirector;
class AudioEventController;
class AudioMic;
class AudioRequestKeeperSyncedBgm;
class AudioResourceDirector;
class AudioSituationDirector;
class BgmKeeper;
class SeListenerKeeper;
class IUseAudioKeeper;
class ModelKeeper;
class PlayerHolder;
class SeEffectController;
class SeKeeper;

class AudioGeneralPurposeAreaChecker : public IUseAreaObj {
public:
    AudioGeneralPurposeAreaChecker(const char* pAreaName);

    void init(AreaObjDirector* pAreaObjDirector);
    void reset();
    void update(s32 islandId);
    bool isInArea() const;
    void setPlayerHolder(const PlayerHolder* pPlayerHolder);
    s32 getIntArgInCurArea(const char* pArgName) const;
    f32 getFloatArgInCurArea(const char* pArgName) const;
    bool getBoolArgInCurArea(const char* pArgName) const;
    const char* getStringArgInCurArea(const char* pArgName) const;
    const char* getStringArgInCurAreaWithAreaCheck(const char* pArgName) const;
    bool tryGetStringArgInCurArea(const char** pArg, const char* pArgName) const;
    bool isCurrAreaCheckForSceneRestart() const;

    AreaObjDirector* getAreaObjDirector() const override { return mAreaObjDirector; }

    const AreaObj* getCurArea() const { return mCurArea; }
    bool isEnteredArea() const { return mIsEnteredArea; }
    bool isExitedArea() const { return mIsExitedArea; }
    bool isAreaChanged() const { return mIsAreaChanged; }

private:
    const char* mAreaName;
    AreaObj* mCurArea = nullptr;
    AreaObj* mPrevArea = nullptr;
    bool _20 = false;
    AreaObjDirector* mAreaObjDirector = nullptr;
    const PlayerHolder* mPlayerHolder = nullptr;
    bool mIsEnteredArea = false;
    bool mIsExitedArea = false;
    bool mIsAreaChanged = false;
};

static_assert(sizeof(AudioGeneralPurposeAreaChecker) == 0x40);

class AudioKeeper {
public:
    AudioKeeper();

    void init(const AudioDirector* pDirector, const char* pSeUserName, const char* pBgmUserName,
              const sead::Vector3f* pTrans, const sead::Matrix34f* pMtx, const ModelKeeper* pModelKeeper,
              const char* pMaterialName);
    void initSeKeeper(const AudioDirector* pDirector, const char* pSeUserName, const sead::Vector3f* pTrans,
                      const sead::Matrix34f* pMtx, const ModelKeeper* pModelKeeper, const char* pMaterialName);
    void initBgmKeeper(const AudioDirector* pDirector, const char* pBgmUserName);
    void initOtherAuido(const AudioDirector* pDirector);
    void update();
    void startClipped();
    void endClipped();
    void appear();
    void kill();

    AudioSituationDirector* getAudioSituationDirector() const { return mAudioSituationDirector; }
    AudioEventController* getAudioEventController() const { return mAudioEventController; }
    SeEffectController* getSeEffectController() const { return mSeEffectController; }
    AudioRequestKeeperSyncedBgm* getAudioRequestKeeperSyncedBgm() const { return mAudioRequestKeeperSyncedBgm; }
    SeKeeper* getSeKeeper() const { return mSeKeeper; }
    BgmKeeper* getBgmKeeper() const { return mBgmKeeper; }
    AudioMic* getAudioMic() const { return mAudioMic; }
    /** @brief Gets the shared SE listener controller. @return Listener keeper, or nullptr before initialization. */
    SeListenerKeeper* getListenerKeeper() const { return mListenerKeeper; }
    IUseAudioKeeper* getUpperLayerAudioUser() const { return mUpperLayerAudioUser; }
    bool isForceInvalidSe() const { return mIsForceInvalidSe; }
    void setIsForceInvalidSe(bool isInvalid) { mIsForceInvalidSe = isInvalid; }

private:
    AudioSituationDirector* mAudioSituationDirector = nullptr;
    AudioEventController* mAudioEventController = nullptr;
    SeEffectController* mSeEffectController = nullptr;
    AudioRequestKeeperSyncedBgm* mAudioRequestKeeperSyncedBgm = nullptr;
    SeKeeper* mSeKeeper = nullptr;
    BgmKeeper* mBgmKeeper = nullptr;
    AudioMic* mAudioMic = nullptr;
    SeListenerKeeper* mListenerKeeper = nullptr;
    IUseAudioKeeper* mUpperLayerAudioUser = nullptr;
    bool mIsForceInvalidSe = false;

};

static_assert(sizeof(AudioKeeper) == 0x50);
}  // namespace al

namespace alAudioHeapFunction {
void createAudioResourceHeapLayer(al::AudioResourceDirector* pDirector, const sead::SafeString& rName);
bool tryCreateAudioResourceHeapLayer(al::AudioResourceDirector* pDirector, const sead::SafeString& rName);
void destroyAudioResourceHeapLayer(al::AudioResourceDirector* pDirector, const sead::SafeString& rName);
bool tryDestroyAudioResourceHeapLayer(al::AudioResourceDirector* pDirector, const sead::SafeString& rName);
bool isExistAudioResourceHeapLayer(al::AudioResourceDirector* pDirector, const sead::SafeString& rName);
}  // namespace alAudioHeapFunction

namespace alAudioKeeperFunction {
al::AudioKeeper* createAndInitAudioKeeper(const al::AudioDirector* pDirector, bool isForceInvalidSe);
}

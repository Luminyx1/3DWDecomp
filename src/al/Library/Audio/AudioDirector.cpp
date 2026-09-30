#include "Library/Audio/AudioDirector.hpp"

#include "Library/Audio/AudioEventController.hpp"
#include "Library/Audio/System/AudioRequestKeeperSyncedBgm.hpp"
#include "Library/Audio/System/AudioSystemInfo.hpp"
#include "Library/Audio/System/AudioVolumeCtrl.hpp"
#include "Library/Bgm/BgmDirector.hpp"
#include "Library/Se/DataBase/SeDataBase.hpp"
#include "Library/Se/Function/SeDirector.hpp"
#include "Library/Se/Function/SeEffectController.hpp"
#include "Library/Se/Info/SeAudioInfo.hpp"
#include "Project/AreaObj/AreaObjGroup.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"
#include "Project/Audio/System/AudioSituationDirector.hpp"

namespace al {
/**
 * Constructs the director and its SE, BGM and volume sub-directors.
 */
AudioDirector::AudioDirector() {
    mSeDirector = new SeDirector();
    mBgmDirector = new BgmDirector();
    mAudioVolumeCtrl = new AudioVolumeCtrl();
}

/**
 * Initializes the sub-directors, the effect controller and the audio event handling.
 * @param pInfo Audio system information.
 * @param pStageName Stage name.
 * @param seRequestNum Number of SE requests.
 * @param unused1 Unused.
 * @param unused2 Unused.
 * @param pBgmStageName BGM stage name.
 * @param unused3 Unused.
 * @param volume Frame rate scale passed to the sub-directors.
 */
void AudioDirector::init(AudioSystemInfo* pInfo, const char* pStageName, s32 seRequestNum, s32 unused1,
                         s32 unused2, const char* pBgmStageName, s32 unused3, f32 volume) {
    mAudioSystemInfo = pInfo;
    mUpperLayerAudioUser = pInfo->mUpperLayerAudioUser;
    mBgmDirector->init(pInfo, pBgmStageName, pStageName, volume);
    mSeDirector->init(pInfo, mBgmDirector->getBgmRhythmCtrl(), seRequestNum, seRequestNum, 60, 40, volume);
    mAudioVolumeCtrl->init();
    mSeEffectController = new SeEffectController();
    mSeEffectController->init(pInfo->getSeadAudioPlayerForSe(), pInfo->getSeadAudioPlayerForBgm(),
                              pInfo->mAudioEffectDataBase);
    mAudioEventController = new AudioEventController(this, pStageName);
    mAudioRequestKeeperSyncedBgm = new AudioRequestKeeperSyncedBgm();
    mAudioRequestKeeperSyncedBgm->init(this);
}

/**
 * Initializes 3D sound and the audio event areas.
 * @param pInfo Audio system information.
 * @param pCameraPos Camera position.
 * @param pCameraMtx Camera matrix.
 * @param pProjection Camera projection.
 * @param pCameraAt Camera look-at position.
 * @param pStageName Stage name.
 * @param pAreaObjDirector Area object director.
 * @param isUseListenerPoser Whether to use listener posers.
 */
void AudioDirector::init3D(const AudioSystemInfo* pInfo, const sead::Vector3f* pCameraPos,
                           const sead::Matrix34f* pCameraMtx, const sead::PerspectiveProjection* pProjection,
                           const sead::Vector3f* pCameraAt, const char* pStageName,
                           AreaObjDirector* pAreaObjDirector, bool isUseListenerPoser) {
    mAreaObjDirector = pAreaObjDirector;
    SeadAudio3DMgr* audio3DMgr = pInfo->mAudio3DMgr;
    mSeDirector->init3D(audio3DMgr, pCameraPos, pCameraMtx, const_cast<sead::PerspectiveProjection*>(pProjection), pCameraAt, pStageName,
                        isUseListenerPoser);
    mAudioEventController->init3D(mAreaObjDirector, mAudioSituationDirector);
    _50 = false;
}

/**
 * Finishes initialization after placement and creates the effect units used by the stage.
 * @param pInfo Audio system information.
 */
void AudioDirector::initAfterInitPlacement(const AudioSystemInfo* pInfo) {
    mAudioEventController->initAfterInitPlacement(this);
    mSeEffectController->createEffectUnit("OuterSpaceAmbLittle");
    AreaObjGroup* group = tryFindAreaObjGroup(this, "AudioEffectChangeArea");
    if (group == nullptr) {
        return;
    }
    for (s32 i = 0; i < group->mNumAreas; i++) {
        AreaObj* areaObj = group->getAreaObj(i);
        const char* effectName = nullptr;
        bool isFound = tryGetAreaObjStringArg(&effectName, areaObj, "AudioEffectName");
        if (effectName != nullptr && isFound) {
            mSeEffectController->createEffectUnit(effectName);
        }
    }
}

/**
 * Creates the situation director and loads its data.
 * @param pCategoryNames SE category names.
 * @param categoryNum Number of categories.
 */
void AudioDirector::initSituationDirector(const char** pCategoryNames, s32 categoryNum) {
    mAudioSituationDirector = new AudioSituationDirector(pCategoryNames, categoryNum);
    mAudioSituationDirector->tryLoadSituationData(SeDataBase::ARC_NAME);
    mSeDirector->initCategoryParamsController(mAudioSituationDirector->getParamsController(0));
}

/**
 * Sets the microphone.
 * @param pMic Microphone.
 */
void AudioDirector::initMic(AudioMic* pMic) {
    mAudioMic = pMic;
}

/**
 * Updates all sub-directors.
 */
void AudioDirector::update() {
    if (mSeEffectController != nullptr) {
        mSeEffectController->update();
    }
    if (mAudioSituationDirector != nullptr) {
        mAudioSituationDirector->update();
    }
    mSeDirector->update();
    mBgmDirector->update();
    mAudioVolumeCtrl->update();
    if (mAudioEventController != nullptr) {
        mAudioEventController->update();
    }
    mAudioRequestKeeperSyncedBgm->update();
}

/**
 * Finalizes the sub-directors and stops all BGM.
 */
void AudioDirector::finalize() {
    if (mSeEffectController != nullptr) {
        mSeEffectController->finalize();
    }
    mSeDirector->finalize();
    mBgmDirector->stopAllBgm(0);
    if (mAudioEventController != nullptr) {
        mAudioEventController->finalize();
    }
}

/**
 * Sets the player holder used by the audio event areas.
 * @param pPlayerHolder Player holder.
 */
void AudioDirector::setPlayerHolder(const PlayerHolder* pPlayerHolder) {
    mAudioEventController->setPlayerHolder(pPlayerHolder);
}

/**
 * Disables BGM changes by area.
 */
void AudioDirector::disableBgmChangeArea() {
    mAudioEventController->setIsDisableBgmChangeArea(true);
}

/**
 * Enables BGM changes by area.
 */
void AudioDirector::enableBgmChangeArea() {
    mAudioEventController->setIsDisableBgmChangeArea(false);
}

/**
 * Sets the default BGM play name.
 * @param pName Play name.
 */
void AudioDirector::setDefaultBgmPlayName(const char* pName) {
    mAudioEventController->setDefaultBgmPlayName(pName);
}

/**
 * Sets the BGM change watcher.
 * @param watcher Watcher id.
 */
void AudioDirector::setBgmChangeWatcher(s32 watcher) {
    mAudioEventController->setBgmChangeWatcher(watcher);
}

/**
 * Makes the next BGM start use the overridden fade-in length.
 */
void AudioDirector::setOverrideFadeInFrames() {
    if (mAudioEventController != nullptr) {
        mAudioEventController->setIsOverrideFadeInFrames(true);
    }
}
}  // namespace al

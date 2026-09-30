#include "Library/Audio/AudioEventController.hpp"

#include <audio/seadAudio3DListenerNin.h>

#include "Library/Audio/AudioDirector.hpp"
#include "Library/Audio/System/AudioKeeper.hpp"
#include "Library/Audio/System/AudioKeeperFunction.hpp"
#include "Library/Bgm/BgmDirector.hpp"
#include "Library/Bgm/BgmFunction.hpp"
#include "Library/Bgm/BgmLineFunction.hpp"
#include "Library/LiveActor/ActorAreaFunction.hpp"
#include "Library/Player/PlayerUtil.hpp"
#include "Library/Se/Function/SeAreaTriggeredPlayer.hpp"
#include "Library/Se/Function/SeDirector.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"
#include "Project/Audio/System/AudioSituationDirector.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
bool isInWaterArea(const IUseAreaObj* pUser, const sead::Vector3f& rPos);
}  // namespace al

namespace {
/**
 * Checks whether more than half of the area target players are in water.
 * @param pUser Area object user.
 * @param pPlayerHolder Player holder.
 * @param pInWaterFrames Frames the players have been in water.
 * @return True if the players are in water.
 */
bool isPlayerInWater(const al::IUseAreaObj* pUser, const al::PlayerHolder* pPlayerHolder, u32* pInWaterFrames) {
    s32 playerNum = al::getPlayerNumMax(pPlayerHolder);
    bool isAnyInWater = false;
    s32 targetNum = 0;
    s32 inWaterNum = 0;
    for (s32 i = 0; i < playerNum; i++) {
        if (al::isPlayerDead(pPlayerHolder, i) || !al::isPlayerAreaTarget(pPlayerHolder, i)) {
            continue;
        }

        targetNum++;
        sead::Vector3f pos = al::getPlayerPos(pPlayerHolder, i);
        if (al::isInWaterAreaNoSink(pUser, pos)) {
            sead::Vector3f upperPos = pos + sead::Vector3f(0.0f, 80.0f, 0.0f);
            if (!al::isInWaterAreaNoSink(pUser, upperPos)) {
                *pInWaterFrames = 0;
                continue;
            }

            if (al::isPlayerInRouteDokan(al::getPlayerActor(pPlayerHolder, i))) {
                return false;
            }

            if (*pInWaterFrames >= 28) {
                inWaterNum++;
                isAnyInWater = true;
            } else {
                (*pInWaterFrames)++;
            }
        } else if (al::isInWaterArea(pUser, pos)) {
            inWaterNum++;
            isAnyInWater = true;
        } else {
            *pInWaterFrames = 0;
        }
    }

    if (targetNum == 0) {
        return false;
    }

    f32 rate = static_cast<f32>(inWaterNum) / static_cast<f32>(targetNum);
    return isAnyInWater && rate > 0.5f;
}

}  // namespace

namespace al {

/**
 * Constructs an audio event controller.
 * @param pDirector Audio director.
 * @param pDefaultAudioEffectName Audio effect used outside of audio effect change areas.
 */
AudioEventController::AudioEventController(const AudioDirector* pDirector, const char* pDefaultAudioEffectName)
    : mDefaultAudioEffectName(pDefaultAudioEffectName) {
    mAudioKeeper = createAudioKeeper(nullptr, pDirector);
    mSeDirector = pDirector->getSeDirector();
}

/**
 * Creates the area checkers for all audio events.
 * @param pAreaObjDirector Area object director.
 * @param pSituationDirector Audio situation director.
 */
void AudioEventController::init3D(AreaObjDirector* pAreaObjDirector, AudioSituationDirector* pSituationDirector) {
    mAreaObjDirector = pAreaObjDirector;
    mBgmChangeWatcher = -1;
    mBgmChangeAreaChecker = new AudioGeneralPurposeAreaChecker("BgmChangeArea");
    mBgmChangeAreaChecker->init(pAreaObjDirector);
    mBgmStartAreaChecker = new AudioGeneralPurposeAreaChecker("BgmStartArea");
    mBgmStartAreaChecker->init(pAreaObjDirector);
    mBgmStopAreaChecker = new AudioGeneralPurposeAreaChecker("BgmStopArea");
    mBgmStopAreaChecker->init(pAreaObjDirector);
    mBgmRegionChangeAreaChecker = new AudioGeneralPurposeAreaChecker("BgmRegionChangeArea");
    mBgmRegionChangeAreaChecker->init(pAreaObjDirector);
    mAudioEffectChangeAreaChecker = new AudioGeneralPurposeAreaChecker("AudioEffectChangeArea");
    mAudioEffectChangeAreaChecker->init(pAreaObjDirector);
    mAudioListenerParamAreaChecker = new AudioGeneralPurposeAreaChecker("AudioListenerParamArea");
    mAudioListenerParamAreaChecker->init(pAreaObjDirector);
    mAudioSituationAreaChecker = new AudioGeneralPurposeAreaChecker("AudioSituationArea");
    mAudioSituationAreaChecker->init(pAreaObjDirector);
    mAudioSituationDirector = pSituationDirector;
}

/**
 * Creates the SE area player if the stage has SE play areas.
 * @param pDirector Audio director.
 */
void AudioEventController::initAfterInitPlacement(const AudioDirector* pDirector) {
    mBgmChangeWatcher = -1;
    if (tryFindAreaObjGroup(this, "SePlayArea") != nullptr) {
        mSeAreaTriggeredPlayer = new SeAreaTriggeredPlayer(pDirector, mAreaObjDirector, mPlayerHolder);
    }
}

/**
 * Updates all enabled audio events.
 */
void AudioEventController::update() {
    if (mEnableEventFlags == 0) {
        return;
    }

    if ((mEnableEventFlags & 1) && mBgmChangeAreaChecker != nullptr) {
        mBgmChangeAreaChecker->update(-1);
        if (mBgmChangeAreaChecker->isAreaChanged()) {
            const char* name = mBgmChangeAreaChecker->getStringArgInCurArea("Kind");
            if (name == nullptr) {
                name = mDefaultBgmPlayName;
            }

            const char* curName = mCurBgmPlayName;
            if (!mIsDisableBgmChangeArea && (name != nullptr || curName != nullptr)) {
                bool isDiffer = true;
                if (name != nullptr && curName != nullptr) {
                    isDiffer = !isEqualString(curName, name);
                }

                if (name != nullptr && isDiffer) {
                    const BgmDataBase* dataBase = getBgmDirector(this)->getBgmDataBase();
                    IUseAudioKeeper* upperUser = getUpperLayerAudioUser(this);
                    alBgmFunction::isPlayingBgmByUpperLayerAudioUser(dataBase, name);
                    const IUseAudioKeeper* user =
                        alBgmFunction::isPlayingBgmByUpperLayerAudioUser(dataBase, name) ? upperUser : this;
                    startBgm(user, name, -1, 0, -1, -1);
                }
            }

            mCurBgmPlayName = name;
        }
    }

    if ((mEnableEventFlags & 0x80) && mBgmStartAreaChecker != nullptr) {
        mBgmStartAreaChecker->update(mBgmChangeWatcher);
        if (mBgmStartAreaChecker->isEnteredArea() || mBgmStartAreaChecker->isAreaChanged()) {
            s32 fadeInFrames = mBgmStartAreaChecker->getIntArgInCurArea("FadeInNewBgmFrame");
            s32 fadeOutFrames = mBgmStartAreaChecker->getIntArgInCurArea("FadeOutCurrentBgmFrame");
            s32 delayFrames = mBgmStartAreaChecker->getIntArgInCurArea("StartDelayFrame");
            s32 startFadeInFrames = fadeInFrames;
            if (mBgmStartAreaChecker->isCurrAreaCheckForSceneRestart() && mIsOverrideFadeInFrames) {
                startFadeInFrames = mOverrideFadeInFrames;
                mIsOverrideFadeInFrames = false;
            }

            const char* name = nullptr;
            mBgmStartAreaChecker->tryGetStringArgInCurArea(&name, "Kind");
            if (name != nullptr) {
                startBgm(this, name, startFadeInFrames, delayFrames, fadeOutFrames, fadeInFrames);
            }
        }
    }

    if ((mEnableEventFlags & 2) && mBgmStopAreaChecker != nullptr) {
        mBgmStopAreaChecker->update(-1);
        if (mBgmStopAreaChecker->isEnteredArea()) {
            s32 fadeOutFrames = mBgmStopAreaChecker->getIntArgInCurArea("FadeOutFrameNum");
            const char* name = nullptr;
            mBgmStopAreaChecker->tryGetStringArgInCurArea(&name, "StopBgmName");
            if (name == nullptr) {
                tryStopAllBgm(this, fadeOutFrames);
            } else {
                stopBgm(this, name, fadeOutFrames, -1);
            }
        }
    }

    if ((mEnableEventFlags & 4) && mBgmRegionChangeAreaChecker != nullptr) {
        mBgmRegionChangeAreaChecker->update(-1);
        if (mBgmRegionChangeAreaChecker->isAreaChanged()) {
            const char* name = mBgmRegionChangeAreaChecker->getStringArgInCurArea("BgmSituationName");
            if (name != nullptr) {
                changeBgmSituation(this, name);
            }
        }
    }

    if ((mEnableEventFlags & 8) && mAudioEffectChangeAreaChecker != nullptr) {
        mAudioEffectChangeAreaChecker->update(-1);
        if (mAudioEffectChangeAreaChecker->isAreaChanged()) {
            const char* name = mAudioEffectChangeAreaChecker->getStringArgInCurArea("AudioEffectName");
            if (name == nullptr) {
                name = mDefaultAudioEffectName;
            }

            changeAudioEffect(this, name);
        }
    }

    if ((mEnableEventFlags & 0x20) && mSeAreaTriggeredPlayer != nullptr) {
        mSeAreaTriggeredPlayer->update();
    }

    const PlayerHolder* playerHolder = mPlayerHolder;
    if (playerHolder != nullptr) {
        bool isInWater = isPlayerInWater(this, playerHolder, &mInWaterFrames);
        if (isInWater) {
            if (!mIsInWater) {
                changeBgmSituation(this, "InWater");
            }
        } else if (mIsInWater) {
            changeBgmSituation(this, "OutWater");
        }

        mIsInWater = isInWater;
    }

    if ((mEnableEventFlags & 0x10) && mAudioListenerParamAreaChecker != nullptr) {
        mAudioListenerParamAreaChecker->update(-1);
        if (mAudioListenerParamAreaChecker->isExitedArea()) {
            mSeDirector->resetListenerParam();
            mSeDirector->changeListenerPoser(nullptr);
        } else if (mAudioListenerParamAreaChecker->isAreaChanged()) {
            sead::Audio3DListenerParameterNin param;
            param.mOutputTypeFlag = 1;
            param.mUserParam = 0;
            param.mInteriorSize = mAudioListenerParamAreaChecker->getFloatArgInCurArea("InteriorSize");
            param.mMaxVolumeDistance = mAudioListenerParamAreaChecker->getFloatArgInCurArea("MaxVolumeDistance");
            param.mUnitDistance = mAudioListenerParamAreaChecker->getFloatArgInCurArea("UnitDistance");
            param.mUnitBiquadFilterValue = -1.0f;
            param.mMaxBiquadFilterValue = -1.0f;
            mSeDirector->changeListenerParam(param);
            const char* poserName = mAudioListenerParamAreaChecker->getStringArgInCurArea("PoserName");
            if (poserName != nullptr && !isEqualString(poserName, "変更なし")) {
                mSeDirector->changeListenerPoser(poserName);
            }
        }
    }

    if ((mEnableEventFlags & 0x40) && mAudioSituationAreaChecker != nullptr) {
        AudioGeneralPurposeAreaChecker* checker = mAudioSituationAreaChecker;
        checker->update(-1);
        if (checker->isExitedArea()) {
            s32 type = checker->getIntArgInCurArea("SituationType");
            mAudioSituationDirector->endSituation(type);
        } else if (checker->isAreaChanged()) {
            s32 type = checker->getIntArgInCurArea("SituationType");
            const char* name = checker->getStringArgInCurArea("SituationName");
            if (name != nullptr && !isEqualString(name, mAudioSituationDirector->getCurrentSituationName(type))) {
                mAudioSituationDirector->startSituation(type, name);
            }
        }
    }
}

/**
 * Checks whether any of the given audio events is enabled.
 * @param type Event flags.
 * @return True if any of the events is enabled.
 */
bool AudioEventController::isEnableAudioEvent(s32 type) {
    return (mEnableEventFlags & type) != 0;
}

/**
 * Does nothing.
 */
void AudioEventController::finalize() {}

/**
 * Sets the player holder used by all area checkers.
 * @param pPlayerHolder Player holder.
 */
void AudioEventController::setPlayerHolder(const PlayerHolder* pPlayerHolder) {
    mPlayerHolder = pPlayerHolder;
    mBgmChangeAreaChecker->setPlayerHolder(mPlayerHolder);
    mBgmStartAreaChecker->setPlayerHolder(mPlayerHolder);
    mBgmStopAreaChecker->setPlayerHolder(mPlayerHolder);
    mBgmRegionChangeAreaChecker->setPlayerHolder(mPlayerHolder);
    mAudioEffectChangeAreaChecker->setPlayerHolder(mPlayerHolder);
    mAudioListenerParamAreaChecker->setPlayerHolder(mPlayerHolder);
    mAudioSituationAreaChecker->setPlayerHolder(mPlayerHolder);
}

/**
 * Enables all audio events.
 */
void AudioEventController::activate() {
    mEnableEventFlags = -1;
}

/**
 * Disables all audio events and resets the area checkers.
 */
void AudioEventController::deactivate() {
    changeAudioEffect(this, nullptr);
    mBgmChangeAreaChecker->reset();
    mBgmStartAreaChecker->reset();
    mBgmStopAreaChecker->reset();
    mBgmRegionChangeAreaChecker->reset();
    mAudioEffectChangeAreaChecker->reset();
    mAudioListenerParamAreaChecker->reset();
    if (mSeAreaTriggeredPlayer != nullptr) {
        mSeAreaTriggeredPlayer->reset();
    }

    mEnableEventFlags = 0;
}

/**
 * Enables the given audio events.
 * @param type Event flags.
 */
void AudioEventController::activateEachAudioEvent(s32 type) {
    mEnableEventFlags |= type;
}

/**
 * Disables the given audio events.
 * @param type Event flags.
 */
void AudioEventController::deactivateEachAudioEvent(s32 type) {
    mEnableEventFlags &= ~type;
}

/**
 * Checks whether player one is in a BGM stop area.
 * @return True if player one is in a BGM stop area.
 */
bool AudioEventController::isInBgmStopArea() {
    if (mBgmStopAreaChecker == nullptr) {
        return false;
    }

    return mBgmStopAreaChecker->isInArea();
}

/**
 * Gets the BGM play name of the BGM change area player one is in.
 * @param isIgnoreDefault Whether to return nullptr instead of the default BGM outside of areas.
 * @return BGM play name.
 */
const char* AudioEventController::getBgmPlayNameByAreaChecker(bool isIgnoreDefault) {
    const char* name = mBgmChangeAreaChecker->getStringArgInCurAreaWithAreaCheck("Kind");
    if (name == nullptr && !isIgnoreDefault) {
        return mDefaultBgmPlayName;
    }

    return name;
}

/**
 * Gets the BGM situation name of the region change area player one is in.
 * @return BGM situation name, or nullptr.
 */
const char* AudioEventController::getBgmSituationNameByAreaChecker() {
    return mBgmRegionChangeAreaChecker->getStringArgInCurAreaWithAreaCheck("BgmSituationName");
}

/**
 * Gets the audio effect name of the audio effect change area player one is in.
 * @return Audio effect name.
 */
const char* AudioEventController::getAudioEffectNameByAreaChecker() {
    const char* name = mAudioEffectChangeAreaChecker->getStringArgInCurAreaWithAreaCheck("AudioEffectName");
    if (name != nullptr) {
        return name;
    }

    return mDefaultAudioEffectName;
}

/**
 * Gets the BGM play name of the BGM area at the given position.
 * @param rPos Position.
 * @return BGM play name.
 */
const char* AudioEventController::getBgmPlayNameInThisPosition(const sead::Vector3f& rPos) {
    AreaObj* areaObj = tryFindAreaObj(this, "BgmChangeArea", rPos);
    if (areaObj == nullptr) {
        areaObj = tryFindAreaObj(this, "BgmStartArea", rPos);
        if (areaObj == nullptr) {
            return mDefaultBgmPlayName;
        }
    }

    const char* name = nullptr;
    bool isFound = tryGetAreaObjStringArg(&name, areaObj, "Kind");
    if (name != nullptr && isFound) {
        return name;
    }

    return mDefaultBgmPlayName;
}

}  // namespace al

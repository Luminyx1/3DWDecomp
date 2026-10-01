#include "Library/Audio/System/AudioKeeper.hpp"

#include "Library/Audio/AudioDirector.hpp"
#include "Library/Bgm/BgmDirector.hpp"
#include "Library/Bgm/BgmKeeper.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Se/Function/SeDirector.hpp"
#include "Library/Se/Project/SeKeeper.hpp"
#include "Project/AreaObj/AreaObj.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"
#include "Project/Audio/System/AudioResourceDirector.hpp"

namespace al {
bool isAreaTarget(const LiveActor* pActor);
s32 getPlayerNumMax(const PlayerHolder* pHolder);
LiveActor* getPlayerActor(const PlayerHolder* pHolder, s32 index);
AreaObj* tryFindAreaObj(const IUseAreaObj* pUser, const char* pName, const sead::Vector3f& rPos);
AreaObj* tryFindAreaObjPlayerOne(const IUseAreaObj* pUser, const char* pName, const PlayerHolder* pHolder);
bool tryGetAreaObjArg(s32* pArg, const AreaObj* pAreaObj, const char* pArgName);
bool tryGetAreaObjArg(f32* pArg, const AreaObj* pAreaObj, const char* pArgName);
}  // namespace al

namespace {
const al::AreaObj* getActiveAreaObj(const al::AreaObj* pAreaObj) {
    if (pAreaObj != nullptr && pAreaObj->mIsValid && !pAreaObj->mIsDisabled) {
        return pAreaObj->_66 ? pAreaObj : nullptr;
    }

    return nullptr;
}
}  // namespace

namespace al {
/**
 * Constructs a checker for areas of the given name.
 * @param pAreaName Area name.
 */
AudioGeneralPurposeAreaChecker::AudioGeneralPurposeAreaChecker(const char* pAreaName)
    : mAreaName(pAreaName) {}

/**
 * Sets the area object director.
 * @param pAreaObjDirector Area object director.
 */
void AudioGeneralPurposeAreaChecker::init(AreaObjDirector* pAreaObjDirector) {
    mAreaObjDirector = pAreaObjDirector;
}

/**
 * Clears the current area and all change flags.
 */
void AudioGeneralPurposeAreaChecker::reset() {
    mIsEnteredArea = false;
    mIsExitedArea = false;
    mIsAreaChanged = false;
    _20 = false;
    mCurArea = nullptr;
    mPrevArea = nullptr;
}

/**
 * Finds the highest priority area the players are in and updates the change flags.
 * @param islandId Current island, or a negative value outside islands.
 */
void AudioGeneralPurposeAreaChecker::update(s32 islandId) {
    _20 = false;
    mIsEnteredArea = false;
    mIsExitedArea = false;
    mIsAreaChanged = false;
    const PlayerHolder* playerHolder = mPlayerHolder;

    if (playerHolder == nullptr) {
        return;
    }

    mPrevArea = mCurArea;
    const char* areaName = mAreaName;
    s32 playerNum = getPlayerNumMax(playerHolder);
    AreaObj* curArea = nullptr;
    bool isOutOfArea = false;

    for (s32 i = 0; i < playerNum; i++) {
        LiveActor* player = getPlayerActor(playerHolder, i);

        if (!isAreaTarget(player) || isDead(player)) {
            continue;
        }

        AreaObj* areaObj = tryFindAreaObj(this, areaName, getTrans(player));

        if (areaObj == nullptr) {
            isOutOfArea = true;
            continue;
        }

        bool isRequireInIsland = false;
        tryGetAreaObjArg(&isRequireInIsland, areaObj, "RequireInIsland");

        if (islandId < 0 && isRequireInIsland) {
            continue;
        }

        if (curArea != nullptr && curArea->getPriority() >= areaObj->getPriority()) {
            continue;
        }

        curArea = areaObj;
    }

    mCurArea = curArea;

    if (!isOutOfArea && curArea == nullptr) {
        return;
    }

    if (mPrevArea != nullptr) {
        mIsEnteredArea = false;
        mIsExitedArea = curArea == nullptr;
    } else {
        mIsEnteredArea = curArea != nullptr;
        mIsExitedArea = false;
    }

    mIsAreaChanged = getActiveAreaObj(curArea) != getActiveAreaObj(mPrevArea);

    if (curArea == nullptr) {
        return;
    }

    bool isOneTime = false;
    tryGetAreaObjArg(&isOneTime, curArea, "IsOneTime");

    if (isOneTime) {
        mCurArea->mIsValid = false;
    }
}

/**
 * Checks whether player one is in an area of this checker.
 * @return True if in an area.
 */
bool AudioGeneralPurposeAreaChecker::isInArea() const {
    if (mAreaName == nullptr || mPlayerHolder == nullptr || mAreaObjDirector == nullptr) {
        return false;
    }

    return tryFindAreaObjPlayerOne(this, mAreaName, mPlayerHolder) != nullptr;
}

/**
 * Sets the player holder.
 * @param pPlayerHolder Player holder.
 */
void AudioGeneralPurposeAreaChecker::setPlayerHolder(const PlayerHolder* pPlayerHolder) {
    mPlayerHolder = pPlayerHolder;
}

/**
 * Gets an integer argument of the current area.
 * @param pArgName Argument name.
 * @return Argument value, or 0.
 */
s32 AudioGeneralPurposeAreaChecker::getIntArgInCurArea(const char* pArgName) const {
    if (mCurArea == nullptr) {
        return 0;
    }

    s32 arg = 0;
    tryGetAreaObjArg(&arg, mCurArea, pArgName);
    return arg;
}

/**
 * Gets a float argument of the current area.
 * @param pArgName Argument name.
 * @return Argument value, or 0.
 */
f32 AudioGeneralPurposeAreaChecker::getFloatArgInCurArea(const char* pArgName) const {
    if (mCurArea == nullptr) {
        return 0.0f;
    }

    f32 arg = 0.0f;
    tryGetAreaObjArg(&arg, mCurArea, pArgName);
    return arg;
}

/**
 * Gets a bool argument of the current area.
 * @param pArgName Argument name.
 * @return True if the argument exists and is set.
 */
bool AudioGeneralPurposeAreaChecker::getBoolArgInCurArea(const char* pArgName) const {
    if (mCurArea == nullptr) {
        return false;
    }

    bool arg = false;
    bool isFound = tryGetAreaObjArg(&arg, mCurArea, pArgName);
    return arg && isFound;
}

/**
 * Gets a string argument of the current area.
 * @param pArgName Argument name.
 * @return Argument value, or nullptr.
 */
const char* AudioGeneralPurposeAreaChecker::getStringArgInCurArea(const char* pArgName) const {
    if (mCurArea == nullptr) {
        return nullptr;
    }

    const char* arg = nullptr;
    tryGetAreaObjStringArg(&arg, mCurArea, pArgName);
    return arg;
}

/**
 * Gets a string argument of the area player one is in.
 * @param pArgName Argument name.
 * @return Argument value, or nullptr.
 */
const char* AudioGeneralPurposeAreaChecker::getStringArgInCurAreaWithAreaCheck(const char* pArgName) const {
    if (pArgName == nullptr) {
        return nullptr;
    }

    AreaObj* areaObj = tryFindAreaObjPlayerOne(this, mAreaName, mPlayerHolder);

    if (areaObj == nullptr) {
        return nullptr;
    }

    const char* arg = nullptr;
    tryGetAreaObjStringArg(&arg, areaObj, pArgName);
    return arg;
}

/**
 * Tries to get a string argument of the current area.
 * @param pArg Receives the argument.
 * @param pArgName Argument name.
 * @return True if found.
 */
bool AudioGeneralPurposeAreaChecker::tryGetStringArgInCurArea(const char** pArg, const char* pArgName) const {
    if (mCurArea == nullptr) {
        return false;
    }

    return tryGetAreaObjStringArg(pArg, mCurArea, pArgName);
}

/**
 * Checks whether the current area is checked again on scene restart.
 * @return True if the CheckForSceneRestart argument is set.
 */
bool AudioGeneralPurposeAreaChecker::isCurrAreaCheckForSceneRestart() const {
    if (mCurArea == nullptr) {
        return false;
    }

    bool arg = false;
    tryGetAreaObjArg(&arg, mCurArea, "CheckForSceneRestart");
    return arg;
}
}  // namespace al

namespace alAudioHeapFunction {
/**
 * Creates an audio resource layer.
 * @param pDirector Resource director.
 * @param rName Layer name.
 */
void createAudioResourceHeapLayer(al::AudioResourceDirector* pDirector, const sead::SafeString& rName) {
    pDirector->createAudioResourceLayer(rName);
}

/**
 * Tries to create an audio resource layer.
 * @param pDirector Resource director.
 * @param rName Layer name.
 * @return True if created.
 */
bool tryCreateAudioResourceHeapLayer(al::AudioResourceDirector* pDirector, const sead::SafeString& rName) {
    return pDirector->tryCreateAudioResourceLayer(rName);
}

/**
 * Destroys an audio resource layer.
 * @param pDirector Resource director.
 * @param rName Layer name.
 */
void destroyAudioResourceHeapLayer(al::AudioResourceDirector* pDirector, const sead::SafeString& rName) {
    pDirector->destroyAudioResourceLayer(rName);
}

/**
 * Tries to destroy an audio resource layer.
 * @param pDirector Resource director.
 * @param rName Layer name.
 * @return True if destroyed.
 */
bool tryDestroyAudioResourceHeapLayer(al::AudioResourceDirector* pDirector, const sead::SafeString& rName) {
    return pDirector->tryDestroyAudioResourceLayer(rName);
}

/**
 * Checks whether an audio resource layer exists.
 * @param pDirector Resource director.
 * @param rName Layer name.
 * @return True if the layer exists.
 */
bool isExistAudioResourceHeapLayer(al::AudioResourceDirector* pDirector, const sead::SafeString& rName) {
    return pDirector->isExistAudioResourceLayer(rName);
}
}  // namespace alAudioHeapFunction

namespace al {
/**
 * Constructs an empty keeper.
 */
AudioKeeper::AudioKeeper() = default;

/**
 * Creates the BGM keeper and, if an SE user name is given, the SE keeper.
 * @param pDirector Audio director.
 * @param pSeUserName SE user name, or nullptr.
 * @param pBgmUserName BGM user name, or nullptr to use the SE user name.
 * @param pTrans Position of the sound source.
 * @param pMtx Matrix of the sound source.
 * @param pModelKeeper Model keeper of the owner.
 * @param pMaterialName Material name.
 */
void AudioKeeper::init(const AudioDirector* pDirector, const char* pSeUserName, const char* pBgmUserName,
                       const sead::Vector3f* pTrans, const sead::Matrix34f* pMtx,
                       const ModelKeeper* pModelKeeper, const char* pMaterialName) {
    AudioSystemInfo* info = pDirector->getAudioSystemInfo();
    mAudioMic = pDirector->getAudioMic();
    mBgmRhythmCtrl = pDirector->getSeDirector()->getBgmRhythmCtrl();
    mSeEffectController = pDirector->getSeEffectController();
    mAudioSituationDirector = pDirector->getAudioSituationDirector();
    mAudioEventController = pDirector->getAudioEventController();
    mAudioRequestKeeperSyncedBgm = pDirector->getAudioRequestKeeperSyncedBgm();

    if (pBgmUserName != nullptr) {
        mBgmKeeper = new BgmKeeper(info, pDirector->getBgmDirector(), pBgmUserName);
    } else {
        mBgmKeeper = new BgmKeeper(info, pDirector->getBgmDirector(), pSeUserName);
    }

    if (pSeUserName != nullptr) {
        mSeKeeper = new SeKeeper(info, pDirector->getSeDirector(), pSeUserName, pTrans, pMtx, pModelKeeper,
                                 pMaterialName);
    }

    mUpperLayerAudioUser = pDirector->getUpperLayerAudioUser();
}

/**
 * Creates the SE keeper.
 * @param pDirector Audio director.
 * @param pSeUserName SE user name, or nullptr.
 * @param pTrans Position of the sound source.
 * @param pMtx Matrix of the sound source.
 * @param pModelKeeper Model keeper of the owner.
 * @param pMaterialName Material name.
 */
void AudioKeeper::initSeKeeper(const AudioDirector* pDirector, const char* pSeUserName,
                               const sead::Vector3f* pTrans, const sead::Matrix34f* pMtx,
                               const ModelKeeper* pModelKeeper, const char* pMaterialName) {
    if (pSeUserName != nullptr) {
        AudioSystemInfo* info = pDirector->getAudioSystemInfo();
        mSeKeeper = new SeKeeper(info, pDirector->getSeDirector(), pSeUserName, pTrans, pMtx, pModelKeeper,
                                 pMaterialName);
    }
}

/**
 * Creates the BGM keeper.
 * @param pDirector Audio director.
 * @param pBgmUserName BGM user name.
 */
void AudioKeeper::initBgmKeeper(const AudioDirector* pDirector, const char* pBgmUserName) {
    AudioSystemInfo* info = pDirector->getAudioSystemInfo();
    mBgmKeeper = new BgmKeeper(info, pDirector->getBgmDirector(), pBgmUserName);
}

/**
 * Takes the shared audio modules from the director.
 * @param pDirector Audio director.
 */
void AudioKeeper::initOtherAuido(const AudioDirector* pDirector) {
    mAudioMic = pDirector->getAudioMic();
    mBgmRhythmCtrl = pDirector->getSeDirector()->getBgmRhythmCtrl();
    mSeEffectController = pDirector->getSeEffectController();
    mAudioSituationDirector = pDirector->getAudioSituationDirector();
    mAudioEventController = pDirector->getAudioEventController();
    mAudioRequestKeeperSyncedBgm = pDirector->getAudioRequestKeeperSyncedBgm();
    mUpperLayerAudioUser = pDirector->getUpperLayerAudioUser();
}

/**
 * Updates the SE keeper.
 */
void AudioKeeper::update() {
    if (mSeKeeper != nullptr) {
        mSeKeeper->update();
    }
}

/**
 * Deactivates the SE keeper while the owner is clipped.
 */
void AudioKeeper::startClipped() {
    if (mSeKeeper != nullptr) {
        mSeKeeper->deactivate(true);
    }
}

/**
 * Reactivates the SE keeper after clipping.
 */
void AudioKeeper::endClipped() {
    if (mSeKeeper != nullptr) {
        mSeKeeper->activate();
    }
}

/**
 * Activates the SE keeper.
 */
void AudioKeeper::appear() {
    if (mSeKeeper != nullptr) {
        mSeKeeper->activate();
    }
}

/**
 * Deactivates the SE keeper.
 */
void AudioKeeper::kill() {
    if (mSeKeeper != nullptr) {
        mSeKeeper->deactivate(false);
    }
}
}  // namespace al

namespace alAudioKeeperFunction {
/**
 * Creates a keeper that only uses the shared audio modules.
 * @param pDirector Audio director.
 * @param isForceInvalidSe Whether SE are always invalid for this keeper.
 * @return New keeper.
 */
al::AudioKeeper* createAndInitAudioKeeper(const al::AudioDirector* pDirector, bool isForceInvalidSe) {
    al::AudioKeeper* keeper = new al::AudioKeeper();
    keeper->initOtherAuido(pDirector);

    if (isForceInvalidSe) {
        keeper->setIsForceInvalidSe(true);
    }

    return keeper;
}
}  // namespace alAudioKeeperFunction

#include "Library/Audio/System/AudioKeeper.hpp"

#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Project/AreaObj/AreaObj.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"

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
        if (curArea != nullptr && curArea->mPriority >= areaObj->mPriority) {
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

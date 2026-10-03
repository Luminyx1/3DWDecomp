#include "Library/Se/Function/SeAreaTriggeredPlayer.hpp"

#include <attributes.h>

#include "Library/Audio/System/AudioKeeper.hpp"
#include "Library/Audio/System/AudioKeeperFunction.hpp"
#include "Library/Player/PlayerUtil.hpp"
#include "Library/Se/Function/SeFunction.hpp"
#include "Library/Se/Project/SeCategory.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/AreaObj/AreaObj.hpp"
#include "Project/AreaObj/AreaObjGroup.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace {
/**
 * @brief Tests whether two areas name the same sound effect.
 * @param pAreaObj First non-null area whose SePlayName argument is read.
 * @param pOther Second non-null area whose SePlayName argument is read.
 * @return True when both names exist and are equal.
 */
bool isSameSePlayArea(const al::AreaObj* pAreaObj, const al::AreaObj* pOther) {
    const char* name = nullptr;
    const char* otherName = nullptr;
    return al::tryGetAreaObjStringArg(&name, pAreaObj, "SePlayName") &&
           al::tryGetAreaObjStringArg(&otherName, pOther, "SePlayName") && name != nullptr &&
           otherName != nullptr && al::isEqualString(name, otherName);
}

/**
 * @brief Finds an area with the same identity or sound-effect name.
 * @param pList Non-null list of areas to search.
 * @param pAreaObj Non-null area to compare against the list entries.
 * @return Matching area, or nullptr if none is found.
 */
const al::AreaObj* findSameSePlayArea(const al::AreaObjArray* pList, const al::AreaObj* pAreaObj) {
    for (s32 i = 0; i < pList->size(); i++) {
        const al::AreaObj* areaObj = pList->unsafeAt(i);

        if (areaObj == pAreaObj || isSameSePlayArea(areaObj, pAreaObj)) {
            return pList->unsafeAt(i);
        }
    }

    return nullptr;
}
} // namespace

namespace al {
/**
 * @brief Constructs the player and its two area lists.
 * @param pDirector Audio director.
 * @param pAreaObjDirector Area object director.
 * @param pPlayerHolder Player holder.
 */
SeAreaTriggeredPlayer::SeAreaTriggeredPlayer(const AudioDirector* pDirector,
                                             AreaObjDirector* pAreaObjDirector,
                                             const PlayerHolder* pPlayerHolder)
    : mAreaObjDirector(pAreaObjDirector), mPlayerHolder(pPlayerHolder) {
    mAudioKeeper = createAudioKeeper("SePlayArea", pDirector);
    mAreaLists = new AreaObjArray*[2];
    mAreaLists[0] = new AreaObjArray();
    mAreaLists[0]->allocBuffer(16, nullptr);
    mAreaLists[1] = new AreaObjArray();
    mAreaLists[1]->allocBuffer(16, nullptr);
}

/**
 * @brief Stops the SE of the areas the players were in and clears the area lists.
 */
void SeAreaTriggeredPlayer::reset() {
    AreaObjArray* prevList = mAreaLists[mCurListIndex > 0 ? mCurListIndex - 1 : 1];

    if (prevList->size() > 0) {
        for (s32 i = 0; i < prevList->size(); i++) {
            const AreaObj* pArea = prevList->unsafeAt(i);
            const char* seName = nullptr;
            tryGetAreaObjStringArg(&seName, pArea, "SePlayName");

            if (seName != nullptr) {
                stopSeByName(this, seName);
            }
        }
    }

    mAreaLists[0]->clear();
    mAreaLists[1]->clear();
    mCurListIndex = 0;
}

/**
 * @brief Starts the SE of areas the players entered and stops the SE of areas they left.
 */
void SeAreaTriggeredPlayer::update() {
    if (mPlayerHolder == nullptr || mAreaObjDirector == nullptr) {
        return;
    }

    AreaObjGroup* group = tryFindAreaObjGroup(this, "SePlayArea");

    if (group == nullptr) {
        return;
    }

    AreaObjArray* curList = mAreaLists[mCurListIndex];
    curList->clear();

    for (s32 i = 0; i < group->getSize(); i++) {
        AreaObj* areaObj = group->getAreaObj(i);
        const PlayerHolder* playerHolder = mPlayerHolder;
        s32 playerNum = getPlayerNumMax(playerHolder);
        bool isInArea = false;

        for (s32 j = 0; j < playerNum; j++) {
            if (isPlayerDead(playerHolder, j) || !isPlayerAreaTarget(playerHolder, j)) {
                continue;
            }

            bool isPlayerInArea = areaObj->isInVolume(getPlayerPos(playerHolder, j));
            isInArea |= isPlayerInArea;

            if (isPlayerInArea) {
                break;
            }
        }

        if (isInArea) {
            curList->pushBack(areaObj);
        }
    }

    AreaObjArray* prevList = mAreaLists[mCurListIndex > 0 ? mCurListIndex - 1 : 1];

    for (s32 i = 0; i < prevList->size(); i++) {
        AreaObj* areaObj = prevList->unsafeAt(i);

        if (findSameSePlayArea(curList, areaObj) != nullptr) {
            continue;
        }

        const char* seName = nullptr;

        if (!tryGetAreaObjStringArg(&seName, areaObj, "SePlayName")) {
            continue;
        }

        bool isDisableSeStop = false;
        tryGetAreaObjArg(&isDisableSeStop, areaObj, "IsDisableSeStop");

        if (!isDisableSeStop) {
            stopSeByName(this, seName);
        }
    }

    for (s32 i = 0; i < curList->size(); i++) {
        AreaObj* areaObj = curList->unsafeAt(i);

        if (findSameSePlayArea(prevList, areaObj) != nullptr) {
            continue;
        }

        const char* seName = nullptr;

        if (tryGetAreaObjStringArg(&seName, areaObj, "SePlayName")) {
            startSeByName(this, seName, nullptr);
        }
    }

    if (mAudioKeeper != nullptr) {
        mAudioKeeper->update();
    }

    mCurListIndex = (mCurListIndex + 1) % 2;
}

/**
 * @brief Constructs the list from category names.
 * @param pNames Category names.
 * @param num Number of categories.
 */
SeCategoryNameList::SeCategoryNameList(const char** pNames, s32 num) {
    mNames.allocBuffer(num, nullptr);

    for (s32 i = 0; i < num; i++) {
        mNames.pushBack(pNames[i]);
    }
}

/**
 * @brief Gets a category name.
 * @param index Category index.
 * @return Category name.
 */
const char* SeCategoryNameList::getCategoryName(s32 index) const { return mNames.unsafeAt(index); }

/**
 * @brief Finds the index of a category.
 * @param pName Category name.
 * @return Category index, or -1.
 */
s32 SeCategoryNameList::findCategoryNoFromName(const char* pName) const {
    for (s32 i = 0; i < mNames.size(); i++) {
        if (isEqualString(mNames.unsafeAt(i), pName)) {
            return i;
        }
    }

    return -1;
}

/**
 * @brief Constructs a volume list with one entry per category.
 * @param pNameList Category names.
 */
SeCategoryInfoList::SeCategoryInfoList(const SeCategoryNameList* pNameList) : mNameList(pNameList) {
    mVolumes.allocBuffer(pNameList->getNum(), nullptr);

    for (s32 i = 0; i < mVolumes.capacity(); i++) {
        mVolumes.pushBack(new f32(0.0f));
    }
}

/**
 * @brief Reads the category volumes from BYAML.
 * @param rIter BYAML iterator with a CategoryVolume dictionary.
 * @return True if the dictionary exists.
 */
bool SeCategoryInfoList::importYaml(ByamlIter& rIter) {
    ByamlIter volumeIter;

    if (!rIter.tryGetIterByKey(&volumeIter, "CategoryVolume")) {
        return false;
    }

    s32 size = volumeIter.getSize();

    for (s32 i = 0; i < size; i++) {
        f32 volume = 0.0f;

        if (!volumeIter.tryGetFloatByIndex(&volume, i)) {
            continue;
        }

        const char* name = nullptr;

        if (!volumeIter.getKeyName(&name, i)) {
            continue;
        }

        for (s32 j = 0; j < mVolumes.size(); j++) {
            if (isEqualString(mNameList->getCategoryName(j), name)) {
                f32* dst = mVolumes.unsafeAt(j);
                *dst = volume;
                break;
            }
        }
    }

    return true;
}

/**
 * @brief Sets the volume of a category.
 * @param pName Category name.
 * @param volume Volume in decibels.
 */
void SeCategoryInfoList::setCategoryVolume(const char* pName, f32 volume) {
    s32 index = mNameList->findCategoryNoFromName(pName);

    if (index >= 0) {
        *mVolumes.unsafeAt(index) = volume;
    }
}

/**
 * @brief Constructs a controller with one mix volume per category.
 * @param pNameList Category names.
 */
SeCategoryParamsController::SeCategoryParamsController(const SeCategoryNameList* pNameList)
    : mNameList(pNameList) {
    mMixVolumes.allocBuffer(pNameList->getNum(), nullptr);

    for (s32 i = 0; i < mMixVolumes.capacity(); i++) {
        mMixVolumes.pushBack(new AudioMixVolume());
    }
}

/**
 * @brief Moves all mix volumes to the volumes of a list.
 * @param pInfoList Target volumes.
 * @param frames Length of the change in frames.
 */
void SeCategoryParamsController::moveTo(const SeCategoryInfoList* pInfoList, s32 frames) {
    for (s32 i = 0; i < mMixVolumes.size(); i++) {
        f32* volume = pInfoList->getVolumes().at(i);
        mMixVolumes.at(i)->moveTo(*volume, frames);
    }
}

/**
 * @brief Updates all mix volumes.
 */
void SeCategoryParamsController::update() {
    for (s32 i = 0; i < mMixVolumes.size(); i++) {
        mMixVolumes.at(i)->update();
    }
}

/**
 * @brief Links each mix volume to the one of another controller.
 * @param pController Controller to link to.
 */
void SeCategoryParamsController::linkTo(const SeCategoryParamsController* pController) {
    for (s32 i = 0; i < mMixVolumes.size(); i++) {
        AudioMixVolume* mixVolume = pController->mMixVolumes.unsafeAt(i);
        mMixVolumes.at(i)->linkTo(mixVolume);
    }
}

/**
 * @brief Gets the mix volume of a category.
 * @param index Category index.
 * @return Mix volume.
 */
AudioMixVolume* SeCategoryParamsController::getMixVolume(s32 index) const {
    return mMixVolumes.unsafeAt(index);
}

/**
 * @brief Constructs a mix volume at 0 dB.
 */
NOINLINE AudioMixVolume::AudioMixVolume() = default;

/**
 * @brief Starts moving the volume.
 * @param volumeDb Target volume in decibels.
 * @param frames Length of the change in frames.
 */
NOINLINE void AudioMixVolume::moveTo(f32 volumeDb, s32 frames) {
    mTargetRatio = calcDecibelToRatio(volumeDb);
    mCurRatio = calcDecibelToRatio(mVolumeDb);
    mRemainFrames = frames;

    if (frames > 0) {
        mStep = (mTargetRatio - mCurRatio) / mRemainFrames;
    } else {
        mStep = 0.0f;
    }
}

/**
 * @brief Advances the volume change.
 */
NOINLINE void AudioMixVolume::update() {
    if (mRemainFrames <= -1.0f) {
        return;
    }

    mRemainFrames += -1.0f;

    if (mRemainFrames <= 0.0f) {
        mVolumeDb = calcRatioToDecibel(mTargetRatio);
        mStep = 0.0f;
        mRemainFrames = -1.0f;
        return;
    }

    mCurRatio += mStep;
    mVolumeDb = calcRatioToDecibel(mCurRatio);
}

/**
 * @brief Links this volume to another one that is added to it.
 * @param pVolume Linked volume.
 */
NOINLINE void AudioMixVolume::linkTo(const AudioMixVolume* pVolume) { mLinkedVolume = pVolume; }

/**
 * @brief Calculates the volume including all linked volumes.
 * @return Volume in decibels.
 */
f32 AudioMixVolume::calcLinkedVolumeDecibel() const {
    if (mLinkedVolume == nullptr) {
        return mVolumeDb;
    }

    return mVolumeDb + mLinkedVolume->calcLinkedVolumeDecibel();
}
} // namespace al

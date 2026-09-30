#include "Library/Se/Function/SeAreaTriggeredPlayer.hpp"

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
bool isSameSePlayArea(const al::AreaObj* pAreaObj, const al::AreaObj* pOther) {
    const char* name = nullptr;
    const char* otherName = nullptr;
    return al::tryGetAreaObjStringArg(&name, pAreaObj, "SePlayName") &&
           al::tryGetAreaObjStringArg(&otherName, pOther, "SePlayName") && name != nullptr &&
           otherName != nullptr && al::isEqualString(name, otherName);
}

const al::AreaObj* findSameSePlayArea(const sead::PtrArray<al::AreaObj>* pList, const al::AreaObj* pAreaObj) {
    for (s32 i = 0; i < pList->size(); i++) {
        const al::AreaObj* areaObj = pList->unsafeAt(i);

        if (areaObj == pAreaObj || isSameSePlayArea(areaObj, pAreaObj)) {
            return pList->unsafeAt(i);
        }
    }

    return nullptr;
}
}  // namespace

namespace al {
/**
 * Constructs the player and its two area lists.
 * @param pDirector Audio director.
 * @param pAreaObjDirector Area object director.
 * @param pPlayerHolder Player holder.
 */
SeAreaTriggeredPlayer::SeAreaTriggeredPlayer(const AudioDirector* pDirector, AreaObjDirector* pAreaObjDirector,
                                             const PlayerHolder* pPlayerHolder)
    : mAreaObjDirector(pAreaObjDirector), mPlayerHolder(pPlayerHolder) {
    mAudioKeeper = createAudioKeeper("SePlayArea", pDirector);
    mAreaLists = new sead::PtrArray<AreaObj>*[2];
    mAreaLists[0] = new sead::PtrArray<AreaObj>();
    mAreaLists[0]->allocBuffer(16, nullptr);
    mAreaLists[1] = new sead::PtrArray<AreaObj>();
    mAreaLists[1]->allocBuffer(16, nullptr);
}

/**
 * Stops the SE of the areas the players were in and clears the area lists.
 */
void SeAreaTriggeredPlayer::reset() {
    sead::PtrArray<AreaObj>* prevList = mAreaLists[mCurListIndex > 0 ? mCurListIndex - 1 : 1];

    for (s32 i = 0; i < prevList->size(); i++) {
        const char* seName = nullptr;
        tryGetAreaObjStringArg(&seName, prevList->unsafeAt(i), "SePlayName");

        if (seName != nullptr) {
            stopSeByName(this, seName);
        }
    }

    mAreaLists[0]->clear();
    mAreaLists[1]->clear();
    mCurListIndex = 0;
}

/**
 * Starts the SE of areas the players entered and stops the SE of areas they left.
 */
void SeAreaTriggeredPlayer::update() {
    if (mPlayerHolder == nullptr || mAreaObjDirector == nullptr) {
        return;
    }

    AreaObjGroup* group = tryFindAreaObjGroup(this, "SePlayArea");

    if (group == nullptr) {
        return;
    }

    sead::PtrArray<AreaObj>* curList = mAreaLists[mCurListIndex];
    curList->clear();

    for (s32 i = 0; i < group->mNumAreas; i++) {
        AreaObj* areaObj = group->getAreaObj(i);
        const PlayerHolder* playerHolder = mPlayerHolder;
        s32 playerNum = getPlayerNumMax(playerHolder);
        bool isInArea = false;

        for (s32 j = 0; j < playerNum; j++) {
            if (isPlayerDead(playerHolder, j) || !isPlayerAreaTarget(playerHolder, j)) {
                continue;
            }

            isInArea |= areaObj->isInVolume(getPlayerPos(playerHolder, j));

            if (isInArea) {
                break;
            }
        }

        if (isInArea) {
            curList->pushBack(areaObj);
        }
    }

    sead::PtrArray<AreaObj>* prevList = mAreaLists[mCurListIndex > 0 ? mCurListIndex - 1 : 1];

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
 * Constructs the list from category names.
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
 * Gets a category name.
 * @param index Category index.
 * @return Category name.
 */
const char* SeCategoryNameList::getCategoryName(s32 index) const {
    return mNames.unsafeAt(index);
}

/**
 * Finds the index of a category.
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
 * Constructs a volume list with one entry per category.
 * @param pNameList Category names.
 */
SeCategoryInfoList::SeCategoryInfoList(const SeCategoryNameList* pNameList) : mNameList(pNameList) {
    mVolumes.allocBuffer(pNameList->getNum(), nullptr);

    for (s32 i = 0; i < mVolumes.capacity(); i++) {
        mVolumes.pushBack(new f32(0.0f));
    }
}

/**
 * Reads the category volumes from BYAML.
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
 * Sets the volume of a category.
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
 * Constructs a controller with one mix volume per category.
 * @param pNameList Category names.
 */
SeCategoryParamsController::SeCategoryParamsController(const SeCategoryNameList* pNameList) : mNameList(pNameList) {
    mMixVolumes.allocBuffer(pNameList->getNum(), nullptr);

    for (s32 i = 0; i < mMixVolumes.capacity(); i++) {
        mMixVolumes.pushBack(new AudioMixVolume());
    }
}

/**
 * Moves all mix volumes to the volumes of a list.
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
 * Updates all mix volumes.
 */
void SeCategoryParamsController::update() {
    for (s32 i = 0; i < mMixVolumes.size(); i++) {
        mMixVolumes.at(i)->update();
    }
}

/**
 * Links each mix volume to the one of another controller.
 * @param pController Controller to link to.
 */
void SeCategoryParamsController::linkTo(const SeCategoryParamsController* pController) {
    for (s32 i = 0; i < mMixVolumes.size(); i++) {
        AudioMixVolume* mixVolume = pController->mMixVolumes.unsafeAt(i);
        mMixVolumes.at(i)->linkTo(mixVolume);
    }
}

/**
 * Gets the mix volume of a category.
 * @param index Category index.
 * @return Mix volume.
 */
AudioMixVolume* SeCategoryParamsController::getMixVolume(s32 index) const {
    return mMixVolumes.unsafeAt(index);
}
}  // namespace al

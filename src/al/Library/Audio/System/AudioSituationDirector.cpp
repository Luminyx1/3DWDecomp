#include "Project/Audio/System/AudioSituationDirector.hpp"

#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Library/Se/Project/SeCategory.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Constructs an empty situation.
 */
AudioSituation::AudioSituation() = default;

/**
 * Reads the situation from a BYAML entry.
 * @param rIter BYAML entry.
 * @param pNameList SE category names.
 */
void AudioSituation::importYaml(ByamlIter& rIter, SeCategoryNameList* pNameList) {
    rIter.tryGetStringByKey(&mName, "SituationName");
    rIter.tryGetIntByKey(&mFadeInFrame, "FadeInFrame");
    rIter.tryGetIntByKey(&mFadeOutFrame, "FadeOutFrame");
    mInfoList = new SeCategoryInfoList(pNameList);
    mInfoList->importYaml(rIter);
}

/**
 * Creates the category names and the linked category parameter controllers.
 * @param pCategoryNames SE category names.
 * @param categoryNum Number of categories.
 */
AudioSituationDirector::AudioSituationDirector(const char** pCategoryNames, s32 categoryNum) {
    mCurrentSituations[0] = nullptr;
    mCurrentSituations[1] = nullptr;
    mCategoryNameList = new SeCategoryNameList(pCategoryNames, categoryNum);
    SeCategoryParamsController* prevController = nullptr;

    for (s32 i = 0; i < mParamsControllers.capacity(); i++) {
        SeCategoryParamsController* controller = new SeCategoryParamsController(mCategoryNameList);
        mParamsControllers.pushBack(controller);

        if (prevController != nullptr) {
            prevController->linkTo(controller);
        }

        prevController = controller;
    }
}

/**
 * Loads the situations of an archive and starts the default situation on both lines.
 * @param pArchiveName Archive name.
 * @return True if the archive has situation data.
 */
bool AudioSituationDirector::tryLoadSituationData(const char* pArchiveName) {
    Resource* resource = findOrCreateResource(pArchiveName, nullptr);
    bool isLoaded;

    if (resource->isExistFile("Situation.byml")) {
        ByamlIter iter(resource->getByml("Situation"));
        s32 size = iter.getSize();

        if (size < 1) {
            return false;
        }

        mSituations.allocBuffer(size, nullptr);

        for (s32 i = 0; i < size; i++) {
            ByamlIter situationIter;
            iter.tryGetIterByIndex(&situationIter, i);
            AudioSituation* situation = new AudioSituation();
            situation->importYaml(situationIter, mCategoryNameList);
            mSituations.pushBack(situation);
        }

        isLoaded = true;
    } else {
        mSituations.allocBuffer(1, nullptr);
        AudioSituation* situation = new AudioSituation();
        situation->mName = "通常";
        mSituations.pushBack(situation);
        situation->mInfoList = new SeCategoryInfoList(mCategoryNameList);
        isLoaded = false;
    }

    startSituation(0, "通常");
    startSituation(1, "通常");
    return isLoaded;
}

/**
 * Starts a situation on a line.
 * @param line Situation line.
 * @param pName Situation name.
 */
void AudioSituationDirector::startSituation(s32 line, const char* pName) {
    AudioSituation* situation = findSituation(pName);

    if (situation == nullptr) {
        return;
    }

    mCurrentSituations[line] = situation;
    mParamsControllers.at(line)->moveTo(situation->getInfoList(), situation->getFadeInFrame());
}

/**
 * Updates the category parameter controllers.
 */
void AudioSituationDirector::update() {
    for (s32 i = 0; i < mParamsControllers.size(); i++) {
        mParamsControllers.at(i)->update();
    }
}

/**
 * Finds a situation by name.
 * @param pName Situation name.
 * @return Situation, or nullptr.
 */
AudioSituation* AudioSituationDirector::findSituation(const char* pName) const {
    for (s32 i = 0; i < mSituations.size(); i++) {
        AudioSituation* situation = mSituations.unsafeAt(i);

        if (isEqualString(situation->getName(), pName)) {
            return situation;
        }
    }

    return nullptr;
}

/**
 * Ends the situation of a line and returns it to the default situation.
 * @param line Situation line.
 */
void AudioSituationDirector::endSituation(s32 line) {
    AudioSituation* situation = findSituation("通常");

    if (situation == nullptr) {
        return;
    }

    AudioSituation* prevSituation = mCurrentSituations[line];
    mCurrentSituations[line] = situation;
    mParamsControllers.at(line)->moveTo(situation->getInfoList(), prevSituation->getFadeOutFrame());
}

/**
 * Gets the name of the current situation of a line.
 * @param line Situation line.
 * @return Situation name.
 */
const char* AudioSituationDirector::getCurrentSituationName(s32 line) const {
    return mCurrentSituations[line]->getName();
}

/**
 * Gets the current situation of a line.
 * @param line Situation line.
 * @return Current situation.
 */
AudioSituation* AudioSituationDirector::getSituationLine(s32 line) const {
    return mCurrentSituations[line];
}
}  // namespace al

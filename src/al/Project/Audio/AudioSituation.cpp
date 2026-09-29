#include "Project/Audio/AudioSituation.hpp"
#include "Library/Yaml/ByamlIter.hpp"

namespace al {
/**
 * @brief Constructs an empty audio situation.
 */
AudioSituation::AudioSituation() = default;

/**
 * @brief Reads the situation's name, fade frames and per-category settings from yaml data.
 * @param rIter The yaml iterator of the situation entry.
 * @param pCategoryNameList The list of sound effect category names the settings refer to.
 */
void AudioSituation::importYaml(ByamlIter& rIter, SeCategoryNameList* pCategoryNameList) {
    rIter.tryGetStringByKey(&mName, "SituationName");
    rIter.tryGetIntByKey(&mFadeInFrame, "FadeInFrame");
    rIter.tryGetIntByKey(&mFadeOutFrame, "FadeOutFrame");
    mCategoryInfoList = new SeCategoryInfoList(pCategoryNameList);
    mCategoryInfoList->importYaml(rIter);
}
}  // namespace al

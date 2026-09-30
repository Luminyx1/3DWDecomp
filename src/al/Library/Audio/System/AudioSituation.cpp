#include "Library/Audio/System/AudioSituation.hpp"

#include "Library/Se/Project/SeCategory.hpp"
#include "Library/Yaml/ByamlIter.hpp"

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

}  // namespace al

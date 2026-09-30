#pragma once

#include <basis/seadTypes.h>

namespace al {
class ByamlIter;
class SeCategoryInfoList;
class SeCategoryNameList;

class AudioSituation {
public:
    AudioSituation();

    void importYaml(ByamlIter& rIter, SeCategoryNameList* pNameList);

    const char* getName() const { return mName; }
    s32 getFadeInFrame() const { return mFadeInFrame; }
    s32 getFadeOutFrame() const { return mFadeOutFrame; }
    const SeCategoryInfoList* getInfoList() const { return mInfoList; }

private:
    const char* mName = nullptr;
    s32 mFadeInFrame = 0;
    s32 mFadeOutFrame = 0;
    SeCategoryInfoList* mInfoList = nullptr;

    friend class AudioSituationDirector;
};
static_assert(sizeof(AudioSituation) == 0x18);
}  // namespace al

#pragma once

#include <basis/seadTypes.h>

namespace al {
class ByamlIter;
class SeCategoryInfoList;
class SeCategoryNameList;

class AudioSituation {
public:
    AudioSituation();

    void importYaml(ByamlIter& rIter, SeCategoryNameList* pCategoryNameList);

    const char* mName = nullptr;                      // _0
    s32 mFadeInFrame = 0;                             // _8
    s32 mFadeOutFrame = 0;                            // _C
    SeCategoryInfoList* mCategoryInfoList = nullptr;  // _10
};

class SeCategoryInfoList {
public:
    SeCategoryInfoList(const SeCategoryNameList* pNameList);

    void importYaml(ByamlIter& rIter);

    void* _0;
    void* _8;
    void* _10;
};
}  // namespace al

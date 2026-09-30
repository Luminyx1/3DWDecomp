#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>

#include "Library/Audio/System/AudioSituation.hpp"

namespace al {
class ByamlIter;
class SeCategoryInfoList;
class SeCategoryNameList;
class SeCategoryParamsController;

class AudioSituationDirector {
public:
    AudioSituationDirector(const char** pCategoryNames, s32 categoryNum);

    bool tryLoadSituationData(const char* pArchiveName);
    void startSituation(s32 line, const char* pName);
    void update();
    AudioSituation* findSituation(const char* pName) const;
    void endSituation(s32 line);
    const char* getCurrentSituationName(s32 line) const;
    AudioSituation* getSituationLine(s32 line) const;

    SeCategoryParamsController* getParamsController(s32 index) const { return mParamsControllers.unsafeAt(index); }

private:
    SeCategoryNameList* mCategoryNameList = nullptr;
    sead::PtrArray<AudioSituation> mSituations;
    AudioSituation* mCurrentSituations[2];
    sead::FixedPtrArray<SeCategoryParamsController, 2> mParamsControllers;
};

static_assert(sizeof(AudioSituationDirector) == 0x48);
}  // namespace al

#pragma once

#include <basis/seadTypes.h>

namespace al {
class StageResourceList;

class StageResourceKeeper {
public:
    StageResourceKeeper();

    void initAndLoadResource(const char* pStageName, s32 scenarioNo);

    StageResourceList* getStageResourceList(s32 index) const { return mStageResourceLists[index]; }
    StageResourceList* getMapStageInfo() const { return mStageResourceLists[0]; }
    StageResourceList* getDesignStageInfo() const { return mStageResourceLists[1]; }
    StageResourceList* getSoundStageInfo() const { return mStageResourceLists[2]; }

    StageResourceList** mStageResourceLists = nullptr;
};
}  // namespace al

#pragma once

#include <basis/seadTypes.h>

namespace al {
class StageResourceList;

class StageResourceKeeper {
public:
    StageResourceKeeper();

    void initAndLoadResource(const char*, s32);

    StageResourceList** mResourceLists = nullptr;  // _0
};
}  // namespace al

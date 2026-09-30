#pragma once

#include <basis/seadTypes.h>

namespace al {
struct ExecuteOrder {
    const char* mListName;
    const char* mExecuteGroup;
    s32 mListMaxSize;
    const char* mGroupType;
    const char* mGroupName;
};

static_assert(sizeof(ExecuteOrder) == 0x28);
}  // namespace al

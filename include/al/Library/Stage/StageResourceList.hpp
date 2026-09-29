#pragma once

#include <container/seadPtrArray.h>
#include <prim/seadSafeString.hpp>

namespace al {
class StageInfo;
struct PlacementInfo;

class StageResourceList {
public:
    StageResourceList(const char*, s32, const char*, bool);

    void initZoneInfo(PlacementInfo&, s32, const char*, PlacementInfo*);
    void initZoneInfoRecursive(const char*, const char*, s32, const char*, const char*,
                               PlacementInfo*);
    s32 getStageResourceNum() const;
    StageInfo* getStageInfo(s32) const;
    StageInfo* findStageInfo(const char*) const;

    sead::PtrArray<StageInfo> mStageInfos;  // _0
    bool mIsExistArchive;                   // _10
    s32 _14;                                // _14
};

void makeStageDataArchivePath(sead::BufferedSafeString*, const char*, s32, const char*, bool);
}  // namespace al
